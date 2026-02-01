import serial
import time
import struct
import threading
from queue import Queue
from crc import Calculator, Configuration

# CRC Configuration
config = Configuration(
    width=8,
    polynomial=0x07,
    init_value=0xFFFFFFFF,
    final_xor_value=0x00000000,
    reverse_input=False,
    reverse_output=False,
)
class Zetta():
    def __init__(self,COM="/dev/ttyACM0",BAUDRATE=115200,TIMEOUT=0.1):
        self.ser = serial.Serial(COM, BAUDRATE, timeout=TIMEOUT)  # Reduced timeout for non-blocking
        self.rx_queue = Queue()  # Queue for received packets
        self.stop_threads = False
        self.calculator = Calculator(config)

        pass
# Global variables
    def parse_packet(self,buf):
        """Parse a received packet"""
        if len(buf) < 6:  # Minimum packet size: start(1) + type(1) + len(1) + crc(1) + stop(1)
            print(f"Packet too short: {len(buf)} bytes")
            return None

        if buf[0] != 0xAA or buf[-1] != 0xBC:
            print(f"Invalid frame: start={hex(buf[0])}, stop={hex(buf[-1])}")
            return None

        pkt_type = buf[1]
        length = buf[2]

        # Check if packet length matches declared length
        if len(buf) != 5 + length:  # start + type + len + payload + crc + stop
            print(f"Length mismatch: declared={length}, actual={len(buf)-5}")
            return None

        payload = buf[3:3+length]
        crc_received = buf[-2]  # CRC is second to last byte

        # Calculate CRC (type + len + payload)
        crc_calculated = self.calculator.checksum(buf[1:3+length]) 

        if crc_received != crc_calculated:
            print(f"CRC mismatch: received={hex(crc_received)}, calculated={hex(crc_calculated)}")
            return None

        print(f"\n[RX] Type: {pkt_type}, Len: {length}, Data: {payload.hex()}")
        return payload 
        # Try to parse structured data
    def create_packet(self,pkt_type, data):
        """Create a packet to send"""
        packet = bytearray()
        packet.append(0xAA)  # START
        packet.append(pkt_type)  # TYPE
        packet.append(len(data))  # LEN
        packet.extend(data)

        # Calculate CRC (type + len + payload)
        crc_data = packet[1:]  # Everything after START byte
        checksum = self.calculator.checksum(crc_data) & 0xFF
        packet.append(checksum)  # CRC
        packet.append(0xBC)  # STOP

        print(f"[TX] Created packet: Type={pkt_type}, Len={len(data)}, CRC={hex(checksum)}")
        return packet

    def receiver_thread(self):
        """Thread for continuous reception"""
        print("Receiver thread started")
        buffer = bytearray()
        state = "WAIT_START"

        while not self.stop_threads:
            # Read all available bytes
            if self.ser.in_waiting > 0:
                data = self.ser.read(self.ser.in_waiting)
                buffer.extend(data)

                # Process buffer
                while len(buffer) > 0:
                    if state == "WAIT_START":
                        # Find START byte
                        if buffer[0] == 0xAA:
                            state = "IN_PACKET"
                        else:
                            buffer.pop(0)  # Discard invalid byte
                            continue
                        
                    if state == "IN_PACKET":
                        # Check if we have enough data for minimal packet
                        if len(buffer) < 5:  # start + type + len + crc + stop
                            break  # Wait for more data
                        
                        length = buffer[2]  # Get length from packet
                        expected_size = 5 + length  # Full packet size

                        if len(buffer) >= expected_size:
                            # Extract complete packet
                            packet = buffer[:expected_size]
                            buffer = buffer[expected_size:]  # Remove processed data

                            # Parse and queue the packet
                            parsed = self.parse_packet(packet)
                            if parsed:
                                self.rx_queue.put(parsed)

                            state = "WAIT_START"  # Reset for next packet
                        else:
                            break  # Not enough data yet
                        
            time.sleep(0.001)  # Small delay to prevent CPU hogging

        print("Receiver thread stopped")

    def process_received_packets(self):
        """Process packets from the receive queue"""
        while not self.rx_queue.empty():
            packet = self.rx_queue.get()
            # You can add custom processing here
            print(f"Processing packet from queue: Type={packet['type']}")
            self.rx_queue.task_done()

def process_user_data(payload,len,pkt_type):
    if len == struct.calcsize("<if5s"):
            try:
                num, price, text = struct.unpack_from("<if5s", payload, 0)
                dict_data = {
                    "type": pkt_type,
                    "number": num,
                    "price": price,
                    "text": text.decode("utf-8", errors="ignore").strip('\x00'),
                }
                print(f"  Parsed: {dict_data}")
            except:
                print("  Could not parse structured data")

            return {"type": pkt_type, "data": payload, "timestamp": time.time()}
    else : 
        print("ERROR: Size Mismatch")

zettap = Zetta() 
periodic_mode = False
periodic_thread = None
def interactive_transmitter():
    """Interactive transmitter in main thread"""
    print("\n=== Interactive Transmitter ===")
    print("Commands:")
    print("  's' - Send structured data")
    print("  't' - Send text data")
    print("  'p' - Send periodic test data (every 2 seconds)")
    print("  'r' - Process received packets")
    print("  'q' - Quit")
    print("===============================\n")
    
    
    def periodic_sender():
        """Send periodic test data"""
        test_count = 0
        while periodic_mode and not zettap.stop_threads:
            test_count += 1
            # Alternate between structured and text data
            if test_count % 2 == 0:
                # Structured data
                data = struct.pack('<if5s', test_count, test_count * 1.5, b"test")
                packet = zettap.create_packet(1, data)  # MSG_PUBLISH = 1
            else:
                # Text data
                text = f"Periodic test {test_count}"
                packet = zettap.create_packet(1, text.encode())
            
            zettap.ser.write(packet)
            print(f"[TX] Sent periodic packet #{test_count}")
            time.sleep(2)
    
    while not zettap.stop_threads:
        command = input("\nEnter command: ").lower()
        
        if command == 'q':
            zettap.stop_threads = True
            if periodic_mode:
                periodic_mode = False
                if periodic_thread:
                    periodic_thread.join()
            break
        
        elif command == 's':
            # Send structured data
            number = int(input("Enter number: "))
            price = float(input("Enter price: "))
            text = input("Enter text (max 5 chars): ")[:5].encode()
            
            data = struct.pack('<if5s', number, price, text)
            packet = zettap.create_packet(1, data)  # MSG_PUBLISH = 1
            zettap.ser.write(packet)
            print("[TX] Structured data sent")
            
        elif command == 't':
            # Send text data
            text = input("Enter text to send: ")
            packet = zettap.create_packet(1, text.encode())  # MSG_PUBLISH = 1
            zettap.ser.write(packet)
            print("[TX] Text data sent")
            
        elif command == 'p':
            # Toggle periodic mode
            periodic_mode = not periodic_mode
            if periodic_mode:
                print("[TX] Starting periodic transmission")
                periodic_thread = threading.Thread(target=periodic_sender, daemon=True)
                periodic_thread.start()
            else:
                print("[TX] Stopped periodic transmission")
                
        elif command == 'r':
            # Process received packets
            print(f"Processing {rx_queue.qsize()} received packets...")
            zettap.process_received_packets()
            
        elif command == '':
            # Just press Enter to send a quick test
            quick_data = struct.pack('<if5s', 42, 99.99, b"quick")
            packet = zettap.create_packet(1, quick_data)
            zettap.ser.write(packet)
            print("[TX] Quick test packet sent")
            
        else:
            print("Unknown command")

def main():
    print("Starting Zetta Protocol Python Test")
    print(f"Connected to {zettap.ser.port} at {zettap.ser.baudrate} baud")
    
    # Start receiver thread
    rx_thread = threading.Thread(target=zettap.receiver_thread, daemon=True)
    rx_thread.start()
    
    # Start interactive transmitter
    try:
        interactive_transmitter()
    except KeyboardInterrupt:
        print("\nInterrupted by user")
    finally:
        # Cleanup
        zettap.stop_threads = True
        rx_thread.join(timeout=1)
        zettap.ser.close()
        print("Serial port closed")
        print("Program terminated")

if __name__ == "__main__":
    main()