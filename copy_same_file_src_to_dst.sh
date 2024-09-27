#!/bin/bash

dst_filenames=`ls $1`
src_filenames=`ls $2`

for dst_file in ${dst_filenames[*]};do
        for src_file in ${src_filenames[*]};do
                if [ "${dst_file}" == "${src_file}" ];then
                        srcfile=$2$src_file
                        dstfile=$1$dst_file
                        cp -rf $srcfile $dstfile
                fi
        done
done
