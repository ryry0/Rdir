#!/bin/sh

function rdir() {
  rdir.x 2> /tmp/rdir
  ERROR=$(</tmp/rdir)
  rm /tmp/rdir
  echo $ERROR
}

function c() {
  rdir.x 2> /tmp/rdir
  ERROR=$(</tmp/rdir)
  rm /tmp/rdir

  echo $ERROR
  DIR=$(echo $ERROR)

  if [[ -d $DIR ]]; then
    builtin cd $DIR
  elif [[ -f $DIR ]]; then
    builtin cd "$(dirname $DIR)"
  fi
}
