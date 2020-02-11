# -----------------------------------------------------------------------------
# Rocks'n'Diamonds for Android
# -----------------------------------------------------------------------------

Add the following content to build Rocks'n'Diamonds for Android:

Download and extract the following SDL libraries to the "jni" directory:

- jni/SDL2
- jni/SDL2_image
- jni/SDL2_mixer
- jni/SDL2_net
- jni/smpeg2

Copy the following repository directories to the "assets" directory:

- assets/conf
- assets/docs
- assets/graphics
- assets/levels
- assets/music
- assets/sounds

Create the file "local.properties" by executing "make ant-init".

When done, use "make" (or "make android" from the repository's root directory)
to compile and package the final Rocks'n'Diamonds APK file.
