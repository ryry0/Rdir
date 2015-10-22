#!/bin/sh

function rdir() {
  rdir.x 2> /tmp/rdir
  ERROR=$(</tmp/rdir)
  rm /tmp/rdir
  echo $ERROR | sed 's/using defaults//'
}

function rcd() {
  rdir.x 2> /tmp/rdir
  ERROR=$(</tmp/rdir)
  rm /tmp/rdir

  echo $ERROR | sed 's/using defaults//'
  DIR=$(echo $ERROR | sed 's/using defaults//')

  if [[ -d $DIR ]]; then
    builtin cd $DIR
  elif [[ -f $DIR ]]; then
    builtin cd "$(dirname $DIR)"
  fi
}
