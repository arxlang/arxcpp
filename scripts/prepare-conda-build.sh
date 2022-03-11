#!/usr/bin/env bash

GIT_ORIGIN=$(git config --get remote.origin.url)
GIT_BRANCH=$(git rev-parse --abbrev-ref HEAD)

GIT_ORIGIN="`echo $GIT_ORIGIN | sed 's%git@%https://%g'`"
GIT_ORIGIN="`echo $GIT_ORIGIN | sed 's%com:%com/%g'`"

rm -rf /tmp/staged-recipes

git clone --depth 1 https://github.com/conda-forge/staged-recipes.git /tmp/staged-recipes
rm -rf /tmp/staged-recipes/recipes/*

cp -R conda/build /tmp/staged-recipes/recipes/arx

# patch
sed -i "s/DOCKER_RUN_ARGS=\"-it\"/DOCKER_RUN_ARGS=\"\"/g" /tmp/staged-recipes/.scripts/run_docker_build.sh
# sed -i "s%{{git_url}}%${GIT_ORIGIN}%g" /tmp/staged-recipes/recipes/arx/meta.yaml
# sed -i "s:{{git_branch}}:${GIT_BRANCH}:g" /tmp/staged-recipes/recipes/arx/meta.yaml

cat /tmp/staged-recipes/recipes/arx/meta.yaml

cp conda/build/linux64.yaml /tmp/staged-recipes/.ci_support
