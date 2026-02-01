import serial
import time 
import struct 
from crc import Calculator, Configuration
config = Configuration(
    width=8,  # CRC width in bits (e.g., 8, 16, 32)
    polynomial=0x07,  # Polynomial used by STM32 hardware
    init_value=0XFFFFFFFF,  # Initial CRC register value
    final_xor_value=0x00000000,  # Value to XOR with final result
    reverse_input=False,  # Set based on your hardware
    reverse_output=False,  # Set based on your hardware
)
calculator = Calculator(config)

ser = serial.Serial("/dev/ttyACM0", 115200, timeout=1)
def parse_packet(buf):
    if buf[0] != 0xAA or buf[-1] != 0xBC:
        print("Invalid frame")
        return
    pkt_start = buf[0] 
    pkt_type = buf[1]
    length   = buf[2]
    payload  = buf[3:3+length]
    crc      = buf[3+length+1]
    stop = buf[-1]

    print("START:", pkt_type)
    print("TYPE:", pkt_type)
    print("LEN:", length)
    print("DATA:", payload.hex())
    print("DATA_STR:", payload.decode(errors="ignore"))
    print("RX CRC:", crc)
    print("Calculate CRC:", calculator.checksum(payload))
    print("STOP:", hex(stop))
    if length  == struct.calcsize("<4sif"):
        name, age, price = struct.unpack_from("<4sif", payload, 0)
        dict = {
            "type": pkt_type,
            "name": name.decode("utf-8").strip("\x00"),
            "age": age,
            "price": price,
        }
        print(dict)
def create_packet(data): 
    packet = bytearray() 
    packet.append(0xAA)
    packet.append(0x01)# type 
    print(f"data len {len(data)}")
    packet.append(len(data))
    packet.extend(data)
    ## CRC 
    checksum = 0
    
    checksum = (calculator.checksum(packet[1:])) 
    print (f"CRC {checksum }")
    checksum = (checksum & 0xff)
    packet.append(checksum) # CRC 
    packet.append(0xBC)# STOP 
    # packet.reverse()
    print(f"packet {packet.hex(' ')}")
    return packet

while True:
    buf = ser.read_until(expected='\xBC')
    if buf:
        print("===================================================================================")
        print("RX:", buf.hex(" "))
        parse_packet(buf)
        print(
            "==================================================================================="
        )
        time.sleep(1)
    # if input("send data: ") == 'y': 
    #     print("sending data ... ")
    #     d= "hello world form pc "
    #     d= bytearray(d.encode()) 
    #     data = ( 10 , 25.2 , b"hello")
    #     d= struct.pack('<if5s',*data)
    #     packet = create_packet(d)
    #     for byte in packet: 
    #         # print(byte)
    #         ser.write(bytes([byte]))
    #     print("packet sent ")
    # else: 
    #     time.sleep(1)
