# To covert txt -> csv, Modify "2csv" file.
- tr $ '\n' < {source_filename} | paste -d, - - - - - - - - - - - > {target_filename}

