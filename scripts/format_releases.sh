#!/usr/bin/env bash
CHANGELOG=build/docs/api/md_CHANGELOG.html

sed -ie 's/&lt;/</' $CHANGELOG
sed -ie 's/&gt;/>/' $CHANGELOG
sed -ie 's/&ndash;/-/' $CHANGELOG
sed -ie 's/&lt;\/a&gt;/<\/a>/' $CHANGELOG
