# -----------------------------------------------------------------------------
# Rocks'n'Diamonds for Android
# -----------------------------------------------------------------------------

# debugging C/ASM:
# arm-linux-androideabi-objdump -S obj/local/armeabi-v7a/libmain.so

PROGBASE = rocksndiamonds

ANDROID_SDK_PATH := ${HOME}/projects/Android/SDK/android-sdk
ANDROID_NDK_PATH := ${HOME}/projects/Android/SDK/android-ndk

SDK_TOOLS_PATH := ${ANDROID_SDK_PATH}/tools
SDK_PLATFORM_TOOLS_PATH := ${ANDROID_SDK_PATH}/platform-tools
NDK_PATH := ${ANDROID_NDK_PATH}

PATH := ${PATH}:${SDK_TOOLS_PATH}:${SDK_PLATFORM_TOOLS_PATH}:${NDK_PATH}

AUTO_GENERATED_DIRS = bin gen obj


all: package

manifest:
	./build-scripts/create_manifest.sh

compile: manifest
	ndk-build NDK_DEBUG=1

verbose: manifest
	ndk-build V=1

package: compile
	ANDROID_AAPT_IGNORE="__IGNORE_NOTHING__" ant debug

install: package
	adb install -r bin/$(PROGBASE)-debug.apk

ant-init:
	android update project --name $(PROGBASE) --path .

assets-toc:
	./build-scripts/create_asset_tocs.sh assets

clean-build:
	ndk-build clean

clean-auto-generated:
	rm -rf $(AUTO_GENERATED_DIRS)

clean: clean-build clean-auto-generated

dist-clean: clean

coredump:
	adb logcat -d | ndk-stack -sym obj/local/armeabi-v7a
