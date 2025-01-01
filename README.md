# Voir

[![Build Voir For Distribution](https://github.com/robrohan/voir/actions/workflows/build.yml/badge.svg)](https://github.com/robrohan/voir/actions/workflows/build.yml)

Voir is a simple application to display images in a terminal. It is a utitlity I use when I am ssh'ed into a server and need a quick idea of what is in an image stored on the server. It tries to resize the image to fit in the current size of the terminal.

For example, in a normal sized terminal, it lets you view an image like this:

![Example](./doc/example.jpg)

## Usage

See `voir -h` for up to date usage. As of this writing:

```bash
voir: basic image viewer for the terminal
Basic usage:
	 voir -i &lt;image&gt;
Options:
	 -a - only use ascii chars (default utf8)
	 -d - use dark mode
	 -c - use colors
	 -h - help screen; this
```

## Building

### Release

```bash
make release_cli
```

```bash
./build/${PLATFORM}/${CPU}/voir -i ./test_data/image1.png
```

### Debug

```bash
make build
```

```bash
./build/${PLATFORM}/${CPU}/voir.debug -i ./test_data/image1.png
```
