import numpy as np
import socket
from scipy.io import wavfile
import math

from scipy.interpolate import interp1d

MESSAGE = np.array([1, 2, 3, 4, 8])
# for value in MESSAGE:
#    print(value)
# MESSAGE = MESSAGE.astype(np.int16)

# socket opening and data sending
UDP_IP = "192.168.0.102"
UDP_PORT = 50007
print("UDP target IP:", UDP_IP)
print("UDP target port:", UDP_PORT)
# print "message:"
sock = socket.socket(socket.AF_INET,  # internet
                     socket.SOCK_DGRAM)  # UDP
# sock.sendto(MESSAGE, (UDP_IP,UDP_PORT)) #Message sending to the established IP and Port

# read the wav file
m = interp1d([0, 65535], [0, pow(2,12) -1])
fs, data = wavfile.read('/media/francisco/OS/rolon_gabo.wav')

number_of_samples = len(data)
number_of_bytes = number_of_samples * 2  # total bytes; each sample has 16bits
max_bytes_per_transfer: int = 1500  # max amount of bytes to be transferred
max_samples_per_transfer: int = int(max_bytes_per_transfer / 2)  # this is because each sample is of 16bits

complete_transfers = math.floor(number_of_bytes / max_bytes_per_transfer)  # the amount of blocks to be transferred
number_of_bytes_final_transfer = number_of_bytes - complete_transfers * max_bytes_per_transfer

complete_transfers = int(complete_transfers)
total_transfers = complete_transfers + 1

Mb = np.array(np.zeros((total_transfers, max_samples_per_transfer)))
# for the blocks of 1500 bytes

offset: int = (pow(2, 16) - 1)/2
n_block: int
for n_block in range(0, complete_transfers - 1):
    for sample in range(0, max_samples_per_transfer - 1):
        Mb[n_block, sample] = np.cast[np.int32](data[(n_block * 750) + sample]) + offset
    # end
# end

number_of_bytes_final_transfer = int(number_of_bytes_final_transfer)
for sample in range(0, int((number_of_bytes_final_transfer / 2) - 1)):
    Mb[n_block, sample] = np.cast[np.int32](data[((n_block + 1) * 750) + sample]) + offset
# end

Ftx = fs / max_samples_per_transfer
Ttx = 1 / Ftx


Mb = m(Mb)
Mb = np.cast[np.uint16](Mb)


# PRUEBA, un solo bloque transmitido
sock.sendto(Mb[0], (UDP_IP, UDP_PORT))  # Message sending to the established IP and Port

# YA TODA LA CANCION (FALTA PROBAR CON UN SOLO BLOQUE)
# cada Ttx, se debe enviar un bloque
# cada envio, debe ser un bloque, es decir MB(indice_A_mover,Enviartodoeste subarray)
# for block in range(0,total_transfers):
#    sock.sendto(Mb[block], (UDP_IP,UDP_PORT)) #Message sending to the established IP and Port
#    time.sleep(Ttx)
# end


# used to play the song via Canopy (used for debugging)
# sd.play(data,fs);
