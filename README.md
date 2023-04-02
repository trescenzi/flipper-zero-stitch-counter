# Flipper Zero Stitch Counter

This simple counter app is intended for use as a knitting stitch counter. The flipper zero is the perfect size for keeping near by while knitting and it's easy enough to increment.

## Building

- Clone [the flipper firmware](https://github.com/flipperdevices/flipperzero-firmware)
- Run `./fbt` in the root of the firmware once to download all the needed packages
- Clone this repo into `flipperzero-firmware/applications_user`
- Run `./ftb stitchcounter`(the appid in application.fam)
- Copy the `.fap` that is produced onto your flipper using qflipper.
  Currently the location is `build/f7-firmware-D/.extapps/stitchcounter.fap` `fbt` will print out the location that it outputs the build



