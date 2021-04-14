# Author: 2017-2018 Alf Gaida <agaida@siduction.org>
# License: WTFPL-2
#  0. You just DO WHAT THE FUCK YOU WANT TO.

# Variables
# =========
# baseversion= the first part of the version no (0.x.y~)
# distribution= target distribution
# repository= your repository
# branch= your branch, if needed
# commit= your commit , if needed
# target_dir= target dir, if needed, default is snapshot
#
# Functions
# =========
# cleanup ()

baseversion=0.7-
distribution=experimental-snapshots
uploadrepo=snapshot

repository="git://anongit.kde.org/trojita.git"

build="yes"
clean="yes"
gitcommit="yes"
push="yes"
upload="yes"
dryrun="no"


# cleanup function definiton
cleanup() {
    echo ""
    echo "┌─────────────────────┐"
    echo "│ Cleanup trojita ... │"
    echo "└─────────────────────┘"
    rm -rf .git*
    rm -rf docs
    rm -rf packaging
    rm -rf qtc_packaging
    rm -rf www

    rm CONTRIBUTING.md
    rm Mainpage.dox
    rm Messages.sh
    rm README
    rm README.localization

    python l10n-fetch-po-files.py
    rm l10n-fetch-po-files.py

    find . -name *.swp -delete
}
