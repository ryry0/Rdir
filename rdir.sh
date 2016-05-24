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


function _rdir_explore() {
  rdir.x 2> /tmp/rdir <$TTY
  ERROR=$(</tmp/rdir)
  rm /tmp/rdir

  DIR=$(echo $ERROR)

  if [[ -d $DIR ]]; then
    builtin cd $DIR
  elif [[ -f $DIR ]]; then
    builtin cd "$(dirname $DIR)"
  fi
  zle reset-prompt
}

function _rdir_sel_widget() {
  rdir.x 2> /tmp/rdir <$TTY
  ERROR=$(</tmp/rdir)
  rm /tmp/rdir

  LBUFFER="${LBUFFER}${ERROR}"
  zle redisplay
}

zle -N _rdir_explore
zle -N _rdir_sel_widget
bindkey '^E' _rdir_explore
bindkey '^F' _rdir_sel_widget
