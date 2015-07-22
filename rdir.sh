#!/bin/sh

function rdir() {
  ./rdir.x 2> /tmp/rdir
  ERROR=$(</tmp/rdir)
  rm /tmp/rdir
  echo $ERROR | sed 's/using defaults//'
}
