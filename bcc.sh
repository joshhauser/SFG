# Check args number
if [ "$#" -eq 0 ]
then
  echo "Too few arguments."
  exit 1
elif [ "$#" -gt 1 ]
then
  echo "Too much arguments"
  exit 1
else
  # Search executable file
  execfileLocation=$(find . -type f -name "$1")

  if [ "$execfileLocation" != "" ]
  then
    # Remove executable file
    rm "$execfileLocation"
  fi

  # Search Makefile
  makefileLocation=$(find . -type f -name "Makefile")

  if [ "$makefileLocation" == "" ]
  then
    echo "No Makefile found"
    exit 1
  fi
  
  # Execute Makefile
  make
fi