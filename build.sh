#!/bin/bash

clear
cd '/boot/home/projects/michi/' || exit 1

if ! command -v hum >/dev/null 2>&1; then
	echo "This build file requires hum." >&2
	exit 127
fi

if [ ! -x /boot/home/projects/ttfx-haiku/ttfx ]; then
	echo "This build file requires TTFX at /boot/home/projects/ttfx-haiku/ttfx." >&2
	exit 127
fi

echo "
▗▖  ▗▖▗▄▄▄▖ ▗▄▄▖▗▖ ▗▖▗▄▄▄▖
▐▛▚▞▜▌  █  ▐▌   ▐▌ ▐▌  █
▐▌  ▐▌  █  ▐▌   ▐▛▀▜▌  █
▐▌  ▐▌▗▄█▄▖▝▚▄▄▖▐▌ ▐▌▗▄█▄▖
	  Builder v0.3 " | /boot/home/projects/ttfx-haiku/ttfx --random-effect
echo ""

if hum confirm "Continue?"; then

	if [ -x /boot/home/config/non-packaged/apps/Shinko ]; then
		/boot/home/config/non-packaged/apps/Shinko "$PWD" &
		shinko_team=$!
		trap '/boot/home/config/non-packaged/apps/Shinko --quit "$shinko_team"' EXIT
	fi

	hum style --foreground "#39ff14" --bold "==> Step 1: Update Code"
	hum spin --title "Updating..." -- git pull
	git show --name-status --oneline

	hum style --foreground "#00f5d4" --bold "==> Step 2: Make Clean"
	make clean

	hum style --foreground "#00bbf9" --bold "==> Step 3: Make"
	make > michi.log 2>&1
	make install

	hum style --foreground "#f15bb5" --bold "==> Step 4: Run / Test"
	if hum confirm "Run?" --default=no; then
		./objects.x86_64-cc15-release/michi
	fi

	if hum confirm "Pack?" --default=no; then

		hum spin --title "Packing..." -- make hpkg
	fi

fi
