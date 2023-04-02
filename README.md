# Flipper Zero Stitch Counter

This simple counter app is intended for use as a knitting stitch counter. The flipper zero is the perfect size for keeping near by while knitting and it's easy enough to increment.

## Example Usage

<img width="430" src="https://user-images.githubusercontent.com/827851/229375555-86a4a2ac-dc6f-4e6c-a20d-f50d5f70303a.gif" />

## Building + Deploying

- Clone [the flipper firmware](https://github.com/flipperdevices/flipperzero-firmware)
- Run `./fbt` in the root of the firmware once to download all the needed packages
- Clone this repo into `flipperzero-firmware/applications_user`
- Run `./ftb stitchcounter`(the appid in application.fam)
- Copy the `.fap` that is produced onto your flipper using qflipper.
  Currently the location is `build/f7-firmware-D/.extapps/stitchcounter.fap` `fbt` will print out the location that it outputs the build

The initial source is forked from [Krulknul/dolphin-counter](https://github.com/Krulknul/dolphin-counter) so many thanks for that great example of a simple app. And this translated [readme was also super helpful](https://github-com.translate.goog/zmactep/flipperzero-hello-world?_x_tr_sl=auto&_x_tr_tl=en&_x_tr_hl=en&_x_tr_pto=wapp).
