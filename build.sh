#/bin/bash


#apt install build-essential
#apt install libglew-dev -y
#apt install libglfw3-dev -y

echo "Building " $1
if [ ${1,,} == 'debug' ]; then
  if cmake -B ./build -S . -DCMAKE_BUILD_TYPE=Debug; then
    if cmake --build ./build -v -t all -j; then
      if dotnet build -c DEBUG /property:GenerateFullPaths=true /property:GenerateFullPaths=true; then
        exit 0
      fi
    fi
  fi
elif [ ${1,,} == 'release' ]; then
  if cmake -B ./build -S . -DCMAKE_BUILD_TYPE=Release; then
    if cmake --build ./build -v -t all -j; then
      if dotnet build -c RELEASE /property:GenerateFullPaths=true /consoleloggerparameters:NoSummary ]; then
        exit 0
      fi
    fi
  fi
else
  echo "invalid configuration supplied $1"
fi
exit 1


