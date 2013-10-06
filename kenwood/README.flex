FlexRadio 6xxx series notes and Hamlib errata by Steve Conklin, AI4QR.

Commands as documented from Flex

I = Implemented
T = TODO
X = Not planned to implement

----------Kenwood Command Set----------
FA Slice receiver A frequency.
FB Slice receiver B frequency.
FR Receive VFO
FT Transmit VFO
IF Transceiver Status
KS Keying speed
MD DSP Mode
RX Set receive (TX off)
SH Set DSP filter high frequency
SL Set DSP filter low frequency
TX Set transmit (TX on)

----------FlexRadio Command Set----------
ZZFA Slice receiver A frequency
ZZFB Slice receiver B frequency
ZZFI Slice Receiver A DSP filter
ZZFJ Slice Receiver B DSP filter
ZZIF Transceiver Status
ZZMD Slice receiver A DSP Mode
ZZME Slice receiver B DSP Mode
ZZSW Set slice Transmit flag (RX A or RX B)
ZZTX Set MOX (on/off)

================================================
rigctl commands used to test operation
(freq set and read)
f
F 14100000
F 14070000

(transceive on to test)
a
A ON

(mode set and read with passband)
m
[returns mode:FM and passband of 13000]
[Command to rig: 'MD', returned 'MD3'

(vfo set and read)
v
V VFOB
[opens with a 5K split]

(set and get split freq)
i
I 14078000
[ Even if split is turned off, this sets VFOB]

(split set and read)
s
[ shows split and VFOA is TX]
S 1 VFOA
s
[Error? Shows split mode still off]
S 1 VFOB
[shows split ON and VFOB TX, but VFOA is still TX on rig]
NOTE: When split off, TX VFO is always A



