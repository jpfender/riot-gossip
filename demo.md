DEMO
====

This will set up a demo network according to the example in the slides.

Dependencies
------------

+ Get desvirt

`git clone https://github.com/des-testbed/desvirt.git`

+ install user-mode-linux and libvirt related desvirt dependencies
+ see [desvirt documentation](https://github.com/des-testbed/desvirt)
+ Debian:

`apt-get install kvm libvirt-bin python-libvirt vlan bridge-utils ebtables uml-utilities tmux`

+ Archlinux:
   
`pacman -S libvirt libvirt-python vlan bridge-utils ebtables tmux`

+ for Archlinux also get uml_utilities_tunpatch

`wget http://blog.galax.is/files/PKGBUILD && makepkg`

Usage
-----

+ build riot-gossip.elf
+ sed path to riot-gossip.elf in demo.xml to your path
+ copy demo.xml to desvirt/.desvirt
+ run
    
`desvirt/vnet -ndemo -d`
and
`desvirt/vnet -ndemo -s`

+ attach the newly spawned tmux and watch all your little elflings

`tmux a`


