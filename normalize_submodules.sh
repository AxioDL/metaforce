#! /bin/bash

origin=$(git remote get-url origin)
origin_base=${origin%/*}

for sub in "extern/amuse" \
           "extern/boo" \
           "extern/jbus" \
           "extern/kabufuda" \
           "extern/nod" \
           "extern/xxhash" \
           "extern/zeus"; do
    if [ -d $sub ]; then
        pushd $sub > /dev/null
        sub_name=$(basename $sub)
        popd > /dev/null
        echo "Changing url for submodule ${sub} to https://github.com/AxioDL/${sub_name}.git"
        git config submodule.$sub.url https://github.com/AxioDL/$sub_name.git
        git submodule init $sub
    fi
done

echo Updating submodules
git submodule update --init --recursive
echo Done
