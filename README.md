SHET C Client
=============

A command line client for [SHET](https://github.com/18sg/SHET/).


Requirements
------------

- [SHETC](https://github.com/18sg/SHET/)
- [json-c](http://oss.metaparadigm.com/json-c/)
- [scons](http://www.scons.org/)


Installation
------------

	$ git clone git://github.com/18sg/SHETCClient.git
	$ cd SHETCClient
	$ scons
	$ sudo scons install

Alternatively, Arch users can install the [shet-c-client-git](https://aur.archlinux.org/packages.php?ID=46040) package from the AUR. Debian users can first switch to Arch, then install the [shet-c-client-git](https://aur.archlinux.org/packages.php?ID=46040) package from the AUR.

Set the shet server to use in your `.bashrc`:

	export SHET_HOST="104.97.120.33"

If you want tab completion:

	source shet_complete


Usage
-----

All parameters are either parsed as JSON, or failing that, used as a string. All output is JSON-formatted. All errors cause the command to exit with a negative return code.

### Calling Actions

	$ shet /tom/notify "hi"
	null

### Getting Properties

	$ shet /tom/audio/input
	1

### Setting Properties

	$ shet /tom/audio/input 1
	null

### Watching Events

	$ shet /lounge/panel/pressed
	[ [ 1, 1 ], [ [ 0, 3 ], false, false ] ]
	[ [ 1, 1 ], [ [ 1, 4 ], false, false ] ]

Each event is printed on it's own line, as an array of parameters.





