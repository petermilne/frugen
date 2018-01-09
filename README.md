### README for frugen

## Generate FRU PROMS for D-TACQ ACQ400 series FMC/ELF modules.

### Setup:
```
cd tools
(cd libipmi; make)
make
mkdir -p fru
```
### Example:
```
[pgm@hoy5 tools]$ ./frugen ACQ427ELF-03-1000-18 42
[]
['ACQ427ELF-03-1000-18', '42']
Generating ACQ427ELF fru/E42730042.fru
```
#### Dump it:
```
pgm@hoy5 tools]$ ./fru-dump fru/E42730042.fru 
fru/E42730042.fru: manufacturer: D-TACQ Solutions
fru/E42730042.fru: product-name: ACQ427ELF
fru/E42730042.fru: serial-number: E42730042
fru/E42730042.fru: part-number: ACQ427ELF-03-1000-18 N=8 M=6C
```
Where
- N=8 : Number of channels = 8
 - ACQ427 is a special case because it may have AO channels as well, additional decode on the -03- field required.
- M=6C : Module type 0x6c (to select FPGA personality)


