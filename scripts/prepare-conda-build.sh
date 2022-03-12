#!/usr/bin/env bash
set -x

PROJECT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && cd .. && pwd )"

GIT_ORIGIN=$(git config --get remote.origin.url)
GIT_BRANCH=$(git rev-parse --abbrev-ref HEAD)

GIT_ORIGIN="`echo $GIT_ORIGIN | sed 's%git@%https://%g'`"
GIT_ORIGIN="`echo $GIT_ORIGIN | sed 's%com:%com/%g'`"

export STAGED_RECIPES=/tmp/staged-recipes

rm -rf ${STAGED_RECIPES}

git clone --depth 1 https://github.com/conda-forge/staged-recipes.git ${STAGED_RECIPES}
rm -rf ${STAGED_RECIPES}/recipes/*

cp -R conda/build ${STAGED_RECIPES}/recipes/arx

# patch
DOCKER_RUN_ARGS="--volume ${PROJECT_DIR}:/tmp/arx"
sed -i "s%DOCKER_RUN_ARGS=\"-it\"%DOCKER_RUN_ARGS=\"${DOCKER_RUN_ARGS}\"%g" ${STAGED_RECIPES}/.scripts/run_docker_build.sh
sed -i "s:#\!/usr/bin/env bash:#\!/usr/bin/env bash\nset -x:g" ${STAGED_RECIPES}/.scripts/run_docker_build.sh
sed -i "s%path: ../..%path: /tmp/arx%g" ${STAGED_RECIPES}/recipes/arx/meta.yaml

python -c 'print("=" * 80)'
cat ${STAGED_RECIPES}/recipes/arx/meta.yaml
python -c 'print("=" * 80)'
cat ${STAGED_RECIPES}/.scripts/run_docker_build.sh
python -c 'print("=" * 80)'

cp conda/build/linux64.yaml ${STAGED_RECIPES}/.ci_support
set +x
