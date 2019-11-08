#!/bin/bash

export CUR_DIR="$(cd -P -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
test1()
{
  ./asort TESTS/file1.txt TESTS/file2.txt
  print "test1 exit code: $(${?})"
  print "hash orig: $(sha256sum ../SORTED/file1.txt file1.sorted)"
  print "hash orig: $(sha256sum ../SORTED/file2.txt file2.sorted)"
}

test2()
{
  "${CUR_DIR}/../asort  FALSE_FILE_ASDDASK FSJ AFSDODSF_FALSE_FILE"
  print "test2 : $(${?})"

}

test3()
{
  "${CUR_DIR}/../asort "
  print "test3 : $(${?})"

}

# I cant of just started running these manually, I didn't have the energy
# to script it right.

# Nyyyyooooooooom
single()
{
  for i in {1..5};
  do
    ./asort "TESTS/file${i}.txt"
    sha256sum "TESTS/file${i}.sorted"
    sha256sum "SORTED/file${i}.txt"
  done
}

all()
{
  ./asort TESTS/file*.txt
  for i in {1..5};
  do
    sha256sum "TESTS/file${i}.sorted"
    sha256sum "SORTED/file${i}.txt"
  done
}

echo "Singles"

single

echo "All"

all

# ZZZzzyyyyyyyoooooooom
