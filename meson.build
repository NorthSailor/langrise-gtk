project('langrise', 'c')

glibdep = dependency('glib-2.0')
gtkdep = dependency('gtk+-3.0')
webkitdep = dependency('webkit2gtk-4.0')
sqldep = dependency('sqlite3')

gnome = import('gnome')

asresources = gnome.compile_resources(
	'as-resources', 'data/gresource.xml',
	source_dir: 'data',
	c_name: 'as'
)

executable('langrise',
	asresources,
	[
		'src/main.c',
		'src/lr-reader.c',
		'src/lr-reader.h',
		'src/lr-database.c',
		'src/lr-database.h',
		'src/lr-language.h',
		'src/lr-language.c',
		'src/lr-main-window.c',
		'src/lr-main-window.h',
		'src/lr-text.h',
		'src/lr-text.c',
		'src/lr-text-selector.c',
		'src/lr-text-selector.h',
	],
	dependencies : [glibdep, gtkdep, sqldep, webkitdep])
