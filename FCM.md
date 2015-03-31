FCM is the movie capture format of FCE-Ultra

## FCM file format description ##
FCM file consists of a variable-length header and various blocks that depend on settings. The header must be a minimum of 56 bytes. The first 32 bytes are interpreted the same between v1 and v2 movies. The header and file layout are completely different between blip's patched version and the unpatched official version of FCEU. Only the savestate and controller data are the same.

### Header format: ###
```
000 4-byte signature: 46 43 4D 1A "FCM\x1A"
004 4-byte little-endian unsigned int: version number, must be 2
008 1-byte flags:
      bit 0: reserved, set to 0
      bit 1:
           if "0", movie begins from an embedded "quicksave" snapshot
           if "1", movie begins from reset or power-on[1]
      bit 2:
           if "0", NTSC timing
           if "1", PAL timing
           see notes below
      other: reserved, set to 0
009 1-byte flags: reserved, set to 0
00A 1-byte flags: reserved, set to 0
00B 1-byte flags: reserved, set to 0
00C 4-byte little-endian unsigned int: number of frames
010 4-byte little-endian unsigned int: rerecord count
014 4-byte little-endian unsigned int: length of controller data in bytes
018 4-byte little-endian unsigned int: offset to the savestate inside file
01C 4-byte little-endian unsigned int: offset to the controller data inside file
020 16-byte md5sum of the ROM used
030 4-byte little-endian unsigned int: version of the emulator used
034 name of the ROM used - UTF8 encoded nul-terminated string.
```

[1](1.md) Even if the header says "movie begins from reset", the file still contains a quicksave, and the quicksave is actually loaded. This flag can't therefore be trusted. To check if the movie actually begins from reset, one must analyze the controller data and see if the first non-idle command in the file is a Reset or Power Cycle type control command.

### Metadata ###
After the header comes "metadata", which is UTF8-coded movie title string. The metadata begins after the ROM name and ends at the savestate offset. This string is displayed as "Author Info" in the Windows version of the emulator.

### Savestate data ###
The savestate offset is <header\_size + length\_of\_metadata\_in\_bytes + padding>. The savestate offset should be 4-byte aligned.
At the savestate offset there is a savestate file.
The savestate exists even if the movie is reset-based.

### Controller Data ###
The controller data offset is <savestate\_offset + length\_of\_compressed\_savestate + padding>. The controller data offset should be 4-byte aligned.
The controller data contains 

<number\_of\_frames>

 frames.

Note that the savestate data must come before controller data.
A gap between the savestate data and the controller data is allowed.

Starting with version 0.98.12 released on September 19, 2004, a PAL flag was added to the header but unfortunately it is not reliable - the emulator does not take the PAL setting from the ROM, but from a user preference. This means that this site cannot calculate movie lengths reliably.

The controller data is written as follows:

| **000** | **001-** |
|:--------|:---------|
| Update byte | (Delta byte(s) if any, max of 3) |

There are two forms of update byte:

```
Control update: 1aabbbbb
aa: Number of delta bytes to follow
bbbbb:  
   0     Do nothing
   1     Reset
   2     Power cycle
   7     VS System Insert Coin
   8     VS System Dipswitch 0 Toggle
  24     FDS Insert
  25     FDS Eject
  26     FDS Select Side
```

```
Controller update: 0aabbccc
aa: Number of delta bytes to follow
bb: Gamepad number minus one (?)
ccc:
  0      A
  1      B
  2      Select
  3      Start
  4      Up
  5      Down
  6      Left
  7      Right
```

The controller update toggles the affected input. Controller update data is emitted to the movie file only when the state of the controller changes.

The delta byte(s) indicate the number of emulator frames between this update and the next update. It is encoded in little-endian format and its size depends on the magnitude of the delta:
```
  Delta of:      Number of bytes:
  0              0
  1-255          1
  256-65535      2
  65536-(2^24-1) 3
```
FCEU emits a "do nothing" command if the current delta is about to overflow.