#!/usr/bin/env bash

compile_retrolite-plymouth-theme() {
	: "${artifact_version:?artifact_version is not set}"

	declare cleanup_id="" tmp_dir=""
	prepare_temp_dir_in_workdir_and_schedule_cleanup "deb-retrolite-plymouth-theme" cleanup_id tmp_dir # namerefs

	declare plymouth_theme_retrolite_dir="retrolite-plymouth-theme"
	mkdir -p "${tmp_dir}/${plymouth_theme_retrolite_dir}"

	run_host_command_logged mkdir -p "${tmp_dir}/${plymouth_theme_retrolite_dir}"/{DEBIAN,usr/share/plymouth/themes/retrolite}

	cd "${tmp_dir}/${plymouth_theme_retrolite_dir}" || exit_with_error "can't change directory"

	# set up control file
	cat <<- END > DEBIAN/control
		Package: retrolite-plymouth-theme
		Version: ${artifact_version}
		Architecture: all
		Maintainer: $MAINTAINER <$MAINTAINERMAIL>
		Depends: plymouth, plymouth-themes
		Section: universe/x11
		Priority: optional
		Description: boot animation, logger and I/O multiplexer - Retro Lite theme
	END

	run_host_command_logged cp "${SRC}"/packages/plymouth-theme-retrolite/debian/{postinst,prerm,postrm} \
		"${tmp_dir}/${plymouth_theme_retrolite_dir}"/DEBIAN/
	chmod 755 "${tmp_dir}/${plymouth_theme_retrolite_dir}"/DEBIAN/{postinst,prerm,postrm}

	# this requires `imagemagick`

	run_host_command_logged convert -resize 256x256 \
		"${SRC}"/packages/plymouth-theme-retrolite/retrolite-logo.png \
		"${tmp_dir}/${plymouth_theme_retrolite_dir}"/usr/share/plymouth/themes/retrolite/bgrt-fallback.png

	run_host_command_logged convert -resize 52x52 \
		"${SRC}"/packages/plymouth-theme-retrolite/spinner.gif \
		"${tmp_dir}/${plymouth_theme_retrolite_dir}"/usr/share/plymouth/themes/retrolite/throbber-%04d.png

	run_host_command_logged cp "${SRC}"/packages/plymouth-theme-retrolite/watermark.png \
		"${tmp_dir}/${plymouth_theme_retrolite_dir}"/usr/share/plymouth/themes/retrolite/

	run_host_command_logged cp "${SRC}"/packages/plymouth-theme-retrolite/{bullet,capslock,entry,keyboard,keymap-render,lock}.png \
		"${tmp_dir}/${plymouth_theme_retrolite_dir}"/usr/share/plymouth/themes/retrolite/

	run_host_command_logged cp "${SRC}"/packages/plymouth-theme-retrolite/retrolite.plymouth \
		"${tmp_dir}/${plymouth_theme_retrolite_dir}"/usr/share/plymouth/themes/retrolite/

	dpkg_deb_build "${tmp_dir}/${plymouth_theme_retrolite_dir}" "retrolite-plymouth-theme"

	done_with_temp_dir "${cleanup_id}" # changes cwd to "${SRC}" and fires the cleanup function early
}
