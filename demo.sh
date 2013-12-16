#!/bin/sh

#change me when needed
cd ../desvirt

./vnet -ndemo -s

tmux join-pane  -s :1
tmux select-layout tiled 
tmux join-pane  -s :2
tmux select-layout tiled
tmux join-pane  -s :3
tmux select-layout tiled
tmux join-pane  -s :4
tmux select-layout tiled
tmux join-pane  -s :5
tmux select-layout tiled
tmux join-pane  -s :6
tmux select-layout tiled
tmux join-pane  -s :7
tmux select-layout tiled
tmux join-pane  -s :8
tmux select-layout tiled
tmux join-pane  -s :9
tmux select-layout tiled

tmux a
