# Rockchip RK3588S octa core 4/8/16GB RAM SoC NVMe USB3 USB-C GbE
BOARD_NAME="Retro Lite CM5"
BOARDFAMILY="rockchip-rk3588"
BOARD_MAINTAINER="ginkage"
BOOTCONFIG="retro-lite-cm5-rk3588s_defconfig"
BOOT_SOC="rk3588"
KERNEL_TARGET="legacy,vendor"
FULL_DESKTOP="yes"
BOOT_LOGO="desktop"
BOOT_FDT_FILE="rockchip/rk3588s-retro-lite-cm5.dtb"
BOOT_SCENARIO="spl-blobs"
BOOT_SUPPORT_SPI="yes"
BOOT_SPI_RKSPI_LOADER="yes"
IMAGE_PARTITION_TABLE="gpt"

function post_family_tweaks__retrolitecm5_naming_audios() {
	display_alert "$BOARD" "Renaming Retro Lite CM5 audios" "info"

	mkdir -p $SDCARD/etc/udev/rules.d/
	echo 'SUBSYSTEM=="sound", ENV{ID_PATH}=="platform-hdmi0-sound", ENV{SOUND_DESCRIPTION}="HDMI0 Audio"' > $SDCARD/etc/udev/rules.d/90-naming-audios.rules
	echo 'SUBSYSTEM=="sound", ENV{ID_PATH}=="platform-dp0-sound", ENV{SOUND_DESCRIPTION}="DP0 Audio"' >> $SDCARD/etc/udev/rules.d/90-naming-audios.rules
	echo 'SUBSYSTEM=="sound", ENV{ID_PATH}=="platform-wm8960-sound", ENV{SOUND_DESCRIPTION}="WM8960 Audio"' >> $SDCARD/etc/udev/rules.d/90-naming-audios.rules

	return 0
}
