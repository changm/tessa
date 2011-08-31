#!/bin/sh
# -*- Mode: sh; indent-tabs-mode: nil; tab-width: 4 -*-
# vi: set ts=4 sw=4 expandtab: (add to ~/.vimrc: set modeline modelines=5)

# usage: addmodeline.sh lang file1 file2 ...
# Add modeline(s) for language to top of each file.
#
# Not clever at all; e.g. leading #! lines, if any, must be restored manually.

# WARNINGS:
# 1. requires $file.orig does not exist (for all $file arguments).
# 2. clobbers leading #! lines.

# Features to-do list:
#
# 1. Rather than require explicit argument specifying language, infer
#    it from extension on filename.  (The case dispatch has been
#    written to accomodate this with relative ease.)
#
# 2. Safe-guard against files that already have modelines.
#    (At the very least, detect them and error in response.)
#
# 3. Safe-guard against files with #! in first line.
#    (At the very least, detect them and error in response;
#     preferably stash the #!-line away and restore to front
#     after modeline has been added.)

MODE_LANG=$1
shift

function add_modeline() {
    file=$1

    if [ ! -e $file ] ; then
        exit 2
    fi
    if [ -e $file.orig ] ; then
        exit 3
    fi

    cp $file $file.orig

    case $MODE_LANG in
        C | c)
            add_modeline_c $file
            ;;
        C++ | c++ | cpp | cxx )
            add_modeline_cxx $file
            ;;
        Java | java )
            add_modeline_java $file
            ;;
        Python | python | py )
            add_modeline_python $file
            ;;
        * )
            echo "Unsupported language: $MODE_LANG"
            exit 1
    esac

    cat $file.orig >> $file
    rm $file.orig
}

function add_modeline_cxx() {
    file_new=$1
    cat > $file_new << "EOF"
/* -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
/* vi: set ts=4 sw=4 expandtab: (add to ~/.vimrc: set modeline modelines=5) */
EOF
}

function add_modeline_c() {
    file_new=$1
    cat <<"EOF" > $file_new
/* -*- Mode: C; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
/* vi: set ts=4 sw=4 expandtab: (add to ~/.vimrc: set modeline modelines=5) */
EOF
}

function add_modeline_java() {
    file_new=$1
    cat > $file_new << "EOF"
/* -*- Mode: Java; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
/* vi: set ts=4 sw=4 expandtab: (add to ~/.vimrc: set modeline modelines=5) */
EOF
}

function add_modeline_python() {
    file_new=$1
    cat > $file_new << "EOF"
#!/usr/bin/env python
# -*- Mode: Python; indent-tabs-mode: nil; tab-width: 4 -*-
# vi: set ts=4 sw=4 expandtab: (add to ~/.vimrc: set modeline modelines=5)
EOF
}

for file in $*; do add_modeline $file ; done
