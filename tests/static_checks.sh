#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)

for file in Constants.h Project.h Project.cpp MainWindow.cpp DeskbarView.cpp MichiApp.cpp; do
	test -f "$root/src/$file"
done

grep -q -- '-std=c++17' "$root/Makefile"
grep -q '^NAME = michi$' "$root/Makefile"
grep -q 'application/x-vnd.michi' "$root/resources/Michi.rdef"
grep -q 'MCHI:created' "$root/src/Project.cpp"
grep -q 'MCHI:modified' "$root/src/Project.cpp"
grep -q 'B_SECONDARY_MOUSE_BUTTON' "$root/src/DeskbarView.cpp"
grep -q 'B_TRASH_DIRECTORY' "$root/src/Project.cpp"

echo "Michi static checks passed"
