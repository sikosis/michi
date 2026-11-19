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
grep -q 'B_USER_NONPACKAGED_DATA_DIRECTORY' "$root/src/Project.cpp"
grep -q 'B_SECONDARY_MOUSE_BUTTON' "$root/src/DeskbarView.cpp"
grep -q 'FillRoundRect' "$root/src/DeskbarView.cpp"
grep -q 'B_ENTERED_VIEW' "$root/src/DeskbarView.cpp"
grep -q 'project%s' "$root/src/DeskbarView.cpp"
grep -q 'B_TRASH_DIRECTORY' "$root/src/Project.cpp"
grep -q 'SetAppHint' "$root/src/MichiApp.cpp"
grep -q 'deskbar.AddItem(&info.ref)' "$root/src/MichiApp.cpp"
grep -q 'deskbar.RemoveItem(kDeskbarItemName)' "$root/src/MichiApp.cpp"
grep -q '_EXPORT BView\* instantiate_deskbar_item' "$root/src/DeskbarView.h"
grep -q '#define MICHI_VERSION "0.3"' "$root/src/Constants.h"
grep -q 'major = 0' "$root/resources/Michi.rdef"
grep -q 'middle = 3' "$root/resources/Michi.rdef"
grep -q 'src/AboutWindow.cpp' "$root/Makefile"
grep -q 'fEmptyStateInitialised' "$root/src/MainWindow.cpp"
grep -q 'Random Colour' "$root/src/ProjectEditor.cpp"
grep -q 'kMsgFilterProjects' "$root/src/MainWindow.cpp"
grep -q 'kFilterUnfinished' "$root/src/MainWindow.cpp"
grep -q 'kFilterNewest' "$root/src/MainWindow.cpp"
grep -q 'src/ProjectReplicantView.cpp' "$root/Makefile"
grep -q 'new ProjectReplicantView' "$root/src/MainWindow.cpp"
grep -q 'BDragger' "$root/src/ProjectReplicantView.cpp"
grep -q 'DrawString("projects"' "$root/src/ProjectReplicantView.cpp"
grep -q 'About michi' "$root/src/DeskbarView.cpp"
grep -q 'B_SECONDARY_MOUSE_BUTTON' "$root/src/MainWindow.cpp"
grep -q 'Small — 2 projects' "$root/src/MainWindow.cpp"
grep -q 'Medium — 5 projects' "$root/src/MainWindow.cpp"
grep -q 'Replicant" B_UTF8_ELLIPSIS' "$root/src/MainWindow.cpp"

echo "Michi static checks passed"
