import numpy as np
import socket
from scipy.io import wavfile
import math
import time
from scipy.interpolate import interp1d

MESSAGE = np.array([1, 2, 3, 4, 8])
# for value in MESSAGE:
#    print(value)
# MESSAGE = MESSAGE.astype(np.int16)

# socket opening and data sending
UDP_IP = "192.168.0.105"
UDP_PORT = 50007
print("UDP target IP:", UDP_IP)
print("UDP target port:", UDP_PORT)
# print "message:"
sock = socket.socket(socket.AF_INET,  # internet
                     socket.SOCK_DGRAM)  # UDP
# sock.sendto(MESSAGE, (UDP_IP,UDP_PORT)) #Message sending to the established IP and Port

# read the wav file
downsample = 1

m = interp1d([0, 65535], [0, pow(2,12) - 100])
fs, data = wavfile.read('/media/francisco/OS/mambo_no_5-lou_bega.wav')
fs = fs/downsample
data = data[0::downsample]

max_samples_per_transfer = 750
number_of_samples = len(data)
number_of_bytes = number_of_samples * 2  # total bytes; each sample has 16bits
max_bytes_per_transfer: int = max_samples_per_transfer*2  # max amount of bytes to be transferred

complete_transfers = math.floor(number_of_bytes / max_bytes_per_transfer)  # the amount of blocks to be transferred
number_of_bytes_final_transfer = number_of_bytes - complete_transfers * max_bytes_per_transfer

complete_transfers = int(complete_transfers)
total_transfers = complete_transfers + 1

Mb = np.array(np.zeros((total_transfers, max_samples_per_transfer)))
# for the blocks of 1500 bytes

offset: int = (pow(2, 16) - 1)/2 +0.5
n_block: int
for n_block in range(0, complete_transfers ):
    for sample in range(0, max_samples_per_transfer ):
        Mb[n_block, sample] = np.cast[np.int32](data[(n_block * max_samples_per_transfer) + sample]) + offset
    # end
# end

number_of_bytes_final_transfer = int(number_of_bytes_final_transfer)
for sample in range(0, int((number_of_bytes_final_transfer / 2) - 1)):
    Mb[n_block, sample] = np.cast[np.int32](data[((n_block + 1) * max_samples_per_transfer) + sample]) + offset
# end

Ftx = fs / max_samples_per_transfer
Ttx = 1 / Ftx

print(Ftx)
print(Ttx)
Mb = m(Mb)
Mb = np.cast[np.uint16](Mb)


# YA TODA LA CANCION (FALTA PROBAR CON UN SOLO BLOQUE)
# cada Ttx, se debe enviar un bloque
# cada envio, debe ser un bloque, es decir MB(indice_A_mover,Enviartodoeste subarray)
block :int

for block in range(0,total_transfers -1):
    sock.sendto(Mb[block], (UDP_IP,UDP_PORT)) #Message sending to the established IP and Port
    time.sleep(Ttx)
# end


# used to play the song via Canopy (used for debugging)
# sd.play(data,fs);
