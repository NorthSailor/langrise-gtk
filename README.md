# Langrise

## What is it?
Langrise is a vocabulary building tool, which, although inspired by LearningWithTexts and LingQ, follows a more systematic approach to storing vocabulary in comparison to those tools.
You can import your texts into Langrise, mark and save any unknown words or phrases, and then create "fill-the-gap" style exercises, in one of many ways.

Texts can be categorized by language, and languages can be fully customized with respect to what is a word and a sentence.

For more information regarding the use of Langrise, consult the Wiki.
If you have previously used either LingQ, LearningWithTexts or a similar tool, you may find the following section useful.

## How does it compare to LearningWithTexts/LingQ?

Langrise differs from these two programs in the following ways:

 * Langrise understands that a single word might correspond to many different series of letters (e.g. knife/knives). Consult the Wiki for more information.
 * Langrise doesn't try to replicate the functionality of external programs; instead it attempts to seamlessly integrate with them. It is meant to be used with programs like Anki for SRS, and GoldenDict for dictionary lookups.
 * Langrise also recognizes that a series of letters may correspond to different words with different meanings, and only a human can know with 100% accuracy which one is the right one in any given context.

The last point, in particular, is the reason why Langrise does not follow LWT's approach of labelling each word as known or unknown, since that would be impossible to do with 100% accuracy.
Instead the user is left to mark the words they are not familiar with.

As an example, the word "ma" in Polish can either correspond to the verb "mieć", or be a shortened form of "moja".
Trying to automatically classify this word could mislead the user into thinking that they know it, while in reality it can have a totally different meaning.

## Installation

At this moment, we only provide pre-built packages for Arch Linux.
If you use a different distribution or OS, you can either build Langrise from scratch, or submit a feature request for your system.

### Distribution packages

#### ArchLinux

You can find the PKGBUILD file under `dist`.
Build and install by running `makepkg -si` in the same directory as the PKGBUILD file.

### Building

#### Dependencies

 * Meson
 * Ninja
 * GTK+ 3.0
 * SQLite

#### Instructions

You can build Langrise like any other meson project:

```
mkdir -p build
cd build
meson ..
ninja install
```

## Lemmatizer packs

A lemmatizer pack, is a massive SQLite database, that maps each word into a lemma.
Currently, lemmatization is only supported for Polish.

### Polish

The lemmatizer is based on the SGJP project.
The database will be distributed via a to-be-announced torrent.

## Contributing

Pull requests are more than welcome.

Before starting to work on a particular issue or feature, please contact me first so that we can coordinate our efforts.

You can help the project immensely by simply packaging it for a new platform.

If nothing else, consider donating:

[PayPal](https://paypal.me/JasonPBar)
Bitcoin: `bitcoin:1P1X3x5uxREJTGBNkcEXpHeuueNYMMcSty`
Litecoin: `MVQeBzRdyGbFLbnxJECh2rtKRUmys7wXkw`

