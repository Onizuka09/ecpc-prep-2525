# interactive_example.py
import time
import struct
import threading
from zetta_protocol import ZettaProtocol, ZettaPacketType, create_struct_parser, create_struct_builder

class InteractiveZetta:
    def __init__(self, port="/dev/ttyACM0"):
        self.zetta = ZettaProtocol(
            port=port,
            baudrate=115200,
            rx_callback=self._on_packet_received
        )
        
        # Register default handlers
        self._register_handlers()
        
        # Periodic transmission
        self.periodic_active = False
        self.periodic_counter = 0
        
    def _register_handlers(self):
        """Register packet handlers for different data types"""
        
        # Structured data handler
        self.zetta.register_packet_handler(
            ZettaPacketType.MSG_PUBLISH,
            parser=lambda data: struct.unpack('<if5s', data),
            builder=lambda data: struct.pack('<if5s', *data)
        )
        
        # Text data handler
        self.zetta.register_packet_handler(
            ZettaPacketType.MSG_ACK,
            parser=lambda data: data.decode('utf-8'),
            builder=lambda data: data.encode('utf-8')
        )
        
        # Raw binary handler (no parser/builder, user handles raw bytes)
        self.zetta.register_packet_handler(
            ZettaPacketType.MSG_SUBSCRIBE,
            parser=None,
            builder=None
        )
    
    def _on_packet_received(self, packet):
        """Callback for received packets"""
        print(f"\n[RX] Packet received: {packet.type.name}")
        print(f"     Timestamp: {packet.timestamp:.3f}")
        print(f"     Data (hex): {packet.data.hex()}")
        
        # Try to parse based on type
        if packet.type == ZettaPacketType.MSG_PUBLISH:
            try:
                num, price, text = struct.unpack('<if5s', packet.data)
                print(f"     Parsed: num={num}, price={price}, text='{text.decode('utf-8')}'")
            except:
                print("     Could not parse as structured data")
        elif packet.type == ZettaPacketType.MSG_ACK:
            try:
                text = packet.data.decode('utf-8', errors='ignore')
                print(f"     Text: '{text}'")
            except:
                print("     Could not decode as text")
    
    def start(self):
        """Start the protocol"""
        self.zetta.start()
        print(f"Zetta Protocol started on {self.zetta.ser.port}")
    
    def stop(self):
        """Stop the protocol"""
        self.zetta.stop()
    
    def send_structured(self):
        """Send structured data"""
        print("\n=== Send Structured Data ===")
        try:
            num = int(input("Enter integer: "))
            price = float(input("Enter float: "))
            text = input("Enter text (max 5 chars): ")[:5].encode()
            
            if self.zetta.send(ZettaPacketType.MSG_PUBLISH, (num, price, text)):
                print("Structured data sent!")
        except ValueError as e:
            print(f"Error: {e}")
    
    def send_text(self):
        """Send text data"""
        print("\n=== Send Text Data ===")
        text = input("Enter text: ")
        if self.zetta.send(ZettaPacketType.MSG_ACK, text):
            print("Text data sent!")
    
    def send_raw(self):
        """Send raw binary data"""
        print("\n=== Send Raw Data ===")
        hex_str = input("Enter hex bytes (e.g., '01 02 03'): ")
        try:
            hex_bytes = hex_str.replace(' ', '')
            if len(hex_bytes) % 2 != 0:
                print("Invalid hex string")
                return
            binary_data = bytes.fromhex(hex_bytes)
            if self.zetta.send_raw(ZettaPacketType.MSG_SUBSCRIBE, binary_data):
                print("Raw data sent!")
        except ValueError as e:
            print(f"Error: {e}")
    
    def toggle_periodic(self):
        """Toggle periodic transmission"""
        self.periodic_active = not self.periodic_active
        if self.periodic_active:
            print("\nStarting periodic transmission...")
            threading.Thread(target=self._periodic_sender, daemon=True).start()
        else:
            print("\nStopping periodic transmission...")
    
    def _periodic_sender(self):
        """Periodic sender thread"""
        while self.periodic_active:
            self.periodic_counter += 1
            
            if self.periodic_counter % 2 == 0:
                # Send structured data
                data = (self.periodic_counter, self.periodic_counter * 1.5, b"test")
                self.zetta.send(ZettaPacketType.MSG_PUBLISH, data)
            else:
                # Send text data
                text = f"Periodic #{self.periodic_counter}"
                self.zetta.send(ZettaPacketType.MSG_ACK, text)
            
            print(f"[Periodic] Sent packet #{self.periodic_counter}")
            time.sleep(2)
    
    def show_stats(self):
        """Show communication statistics"""
        stats = self.zetta.get_stats()
        print("\n=== Statistics ===")
        for key, value in stats.items():
            print(f"{key:20}: {value}")
    
    def interactive_menu(self):
        """Interactive menu for testing"""
        self.start()
        
        try:
            while True:
                print("\n" + "="*50)
                print("ZETTA PROTOCOL INTERACTIVE TEST")
                print("="*50)
                print("1. Send structured data (int, float, 5-byte string)")
                print("2. Send text data")
                print("3. Send raw binary data")
                print("4. Toggle periodic transmission")
                print("5. Show statistics")
                print("6. Flush receive queue")
                print("7. Exit")
                print("-"*50)
                
                choice = input("Select option (1-7): ").strip()
                
                if choice == '1':
                    self.send_structured()
                elif choice == '2':
                    self.send_text()
                elif choice == '3':
                    self.send_raw()
                elif choice == '4':
                    self.toggle_periodic()
                elif choice == '5':
                    self.show_stats()
                elif choice == '6':
                    self.zetta.flush()
                    print("Receive queue flushed")
                elif choice == '7':
                    break
                else:
                    print("Invalid choice")
                
                # Process any pending packets
                self._process_pending_packets()
                
        except KeyboardInterrupt:
            print("\nInterrupted by user")
        finally:
            self.stop()
    
    def _process_pending_packets(self):
        """Process any packets in the queue"""
        while True:
            packet = self.zetta.get_packet(timeout=0)
            if packet:
                self._on_packet_received(packet)
            else:
                break

if __name__ == "__main__":
    # User can easily customize the port
    app = InteractiveZetta(port="/dev/ttyACM0")  # Change to your port
    app.interactive_menu()