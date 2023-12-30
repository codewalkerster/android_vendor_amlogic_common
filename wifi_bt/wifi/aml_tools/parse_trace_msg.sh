#!/bin/bash  

# file name
file_path="fmacfw.pm"  
  
if [ ! -f "$file_path" ]; then  
  echo "file not exist: $file_path"  
  exit 1  
fi  
  
cat "$file_path"
