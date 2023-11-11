#!/bin/sh

git ls-tree -r HEAD --name-only | grep -E '\.(cc|h)$' | uncrustify -c uncrustify.cfg --no-backup --replace -F -
