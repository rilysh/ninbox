#!/bin/zsh

function _ninbox() {
    local _values

    _values=(
	'--load-name' '--load-num'
	'--edit-name' '--edit-num'
	'--del-name' '--del-num'
	'--view-name' '--view-num'
	'--list' '--rom-path'
	'--root-path' '--settings'
    )
    compadd -a _values
}

compdef _ninbox ninbox
