#!/bin/sh
#phpdoc HTML:frames:earthli -f index.php -t docs
#phpdoc HTML:frames:l0l33t -d client,server -t docs
#phpdoc -o HTML:frames:DOM/earthli -d islandora -t docs
phpdoc -i xml,xsl -dn "Islandora" -ti "Islandora" -o HTML:frames:DOM/earthli -d . -t docs


