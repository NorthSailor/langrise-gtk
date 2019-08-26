project('langrise', 'c')

glibdep = dependency('glib-2.0')
gtkdep = dependency('gtk+-3.0')
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
		'src/lr-db-lemmatizer.c',
		'src/lr-db-lemmatizer.h',
		'src/lr-language.c',
		'src/lr-language.h',
		'src/lr-language-editor-dialog.c',
		'src/lr-language-editor-dialog.h',
		'src/lr-language-manager-dialog.c',
		'src/lr-language-manager-dialog.h',
		'src/lr-lemma.c',
		'src/lr-lemma.h',
		'src/lr-lemma-instance.c',
		'src/lr-lemma-instance.h',
		'src/lr-lemmatizer.c',
		'src/lr-lemmatizer.h',
		'src/lr-lemma-suggestion.c',
		'src/lr-lemma-suggestion.h',
		'src/lr-main-window.c',
		'src/lr-main-window.h',
		'src/lr-reader.c',
		'src/lr-reader.h',
		'src/lr-splitter.c',
		'src/lr-splitter.h',
		'src/lr-text.h',
		'src/lr-text.c',
		'src/lr-text-dialog.c',
		'src/lr-text-dialog.h',
		'src/lr-text-selector.c',
		'src/lr-text-selector.h',
	],
	dependencies : [glibdep, gtkdep, sqldep])
