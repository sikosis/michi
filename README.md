# Michi

Michi is a small native Haiku project-progress tracker with a Deskbar replicant.

Projects are stored as individual files with BFS attributes. The application is
written against the Haiku API in C++17.

## Build

On Haiku, run:

```sh
make
```

Launch `michi` once to install its Deskbar item. Closing the main window leaves
the Deskbar item installed; choose **Quit** from its context menu to remove it.

## Project data

Michi keeps one file per project in the user's Haiku data directory under
`Michi/Projects`. Notes are the UTF-8 file contents; the name, progress,
colour, created time, and modified time use `MCHI:*` BFS attributes. Settings
are stored as a flattened `BMessage` in the user's settings directory.

The default ordering is modified time, oldest first. Settings can switch
between created or modified time and reverse the direction.

The SVG in `artwork/` is the editable icon source. The Deskbar item draws the
same winding-path mark with native Haiku drawing calls so it remains sharp at
any Deskbar size.

