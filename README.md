org.quietmodem.Quiet
===========

[![Quiet Modem Chat](https://discordapp.com/api/guilds/290985648054206464/embed.png?style=shield)](https://discordapp.com/invite/eRw5UjF)

org.quietmodem.Quiet allows you to pass data through the speakers on your Android device. This branch operate *ONLY* as a raw frame layer.

This package contains prebuilt library files for [libquiet](https://github.com/quiet/quiet) as well as their dependencies. On top of that, it adds Java bindings which closely mimic the familiar interfaces from the java.net.* package.

This package is provided under the 3-clause BSD license. The licenses of its dependencies are also included and are licensed under a mix of BSD and MIT.

Quiet comes with support for armeabi-v7a, arm64-v8a. It requires Android API 14 for 32-bit mode and API 21 for 64-bit mode. It requires only the `RECORD_AUDIO` permission.

For testing purposes, Genymotion is highly recommended over the default emulator. Genymotion provides access to the microphone while the default Android Studio one does not and will throw an exception when Quiet attempts to use the microphone.

Important note:
---------------
This branch is based off msequence_reodering and I have removed support for x86 and x86_64.Support for lwip has also been removed to slim down the code base. 

Why sound? Isn't that outdated?
---------------
If you are old enough, you may remember using dial-up modems to connect to the internet. In a sense, this package brings that back. While it's true that this is somewhat of a retro approach, consider the advantages of using sound.

* Highly cross-platform. Any device with speakers and a microphone and sufficient computational power can use this medium to communicate.

* No pairing. Unlike Bluetooth, sound can be used instantly without the need to pair devices. This reduces the friction and improves the user experience.

* Embeddable content. Similar to a QR code, short packets of data can be encoded into streaming or recorded audio and can then be later decoded by this package.

What does it sound like?
---------------
The answer to this depends on which operating mode you choose. Quiet provides audible and near-ultrasonic modes. Audible modes sound something like a puff of air. The near-ultrasonic modes run at 17+kHz and are virtually inaudible to adults. Either mode can operate at relatively low volumes as long as there isn't too much background noise.

How fast does it go?
---------------
Quiet's provided audible mode transfers at approximately 7kbps. In cases where two devices are connected over a cable (via 3.5mm jack) it can run in cable mode, which transfers at approximately 64kbps.

Other Platforms
---------------

Desktop/Laptop: [libquiet](https://github.com/quiet/quiet)

Javascript: [quiet-js](https://github.com/quiet/quiet-js) (*UDP/TCP coming soon*)

iOS: [QuietModemKit](https://github.com/quiet/QuietModemKit)

Apps that use org.quietmodem.Quiet
----------------------------------

[Quiet Share](https://github.com/alexbirkett/QuietShare) - A proof of concept app that allows text and links to be shared from one device to another.

Usage
===========

Quiet can be used *ONLY* as a raw frame layer.

Make sure to have the [Android NDK](https://developer.android.com/ndk/index.html) installed and set the location of it at `ndk.dir` in `local.properties`. This is necessary to build the JNI wrapper included in this project.

Frame Mode
---------------
Assuming we're working from MainActivity.java, we start with

```
import java.io.IOException;
import org.quietmodem.Quiet.*;

FrameReceiverConfig receiverConfig = null;
FrameTransmitterConfig transmitterConfig = null;

try {
    transmitterConfig = new FrameTransmitterConfig(
            this,
            "audible-7k-channel-0");
    receiverConfig = new FrameReceiverConfig(
            this,
            "audible-7k-channel-0");
} catch (IOException e) {
    // could not build configs
}

FrameReceiver receiver = null;
FrameTransmitter transmitter = null;

try {
    receiver = new FrameReceiver(receiverConfig);
    transmitter = new FrameTransmitter(transmitterConfig);
} catch (ModemException e) {
    // could not set up receiver/transmitter
}

```

This sets up our transmitter and receiver using the packaged configuration. We choose the audible mode here. Now we can transmit.

On one side we might run
```
// set receiver to block until a frame is received
// by default receivers are nonblocking
receiver.setBlocking(0, 0);

byte[] buf = new byte[1024];
long recvLen = 0;
try {
    recvLen = receiver.receive(buf);
} catch (IOException e) {
    // read timed out
}
```

And on the other side
```
String payload = "Hello, World!";
try {
    transmitter.send(payload.getBytes());
} catch (IOException e) {
    // our message might be too long or the transmit queue full
}
```

That's enough to send our frame across. Frame mode is useful when we want to send small bits of data that can easily fit in one frame and do not need a concept of a sender or receiver, that is, frames are a broadcast medium.
