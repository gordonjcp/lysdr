# This is an example PKGBUILD file. Use this as a start to creating your own,
# and remove these comments. For more information, see 'man PKGBUILD'.
# NOTE: Please fill out the license field for your package! If it is unknown,
# then please put 'unknown'.

# See http://wiki.archlinux.org/index.php/VCS_PKGBUILD_Guidelines
# for more information on packaging from GIT sources.

# Maintainer: Gordonjcp <gordon@gjcp.net>
pkgname=lysdr-git
pkgver=0.0.6
pkgrel=1
pkgdesc="Software-defined Radio receiver"
arch=(i686)
url="http://lovesthepython.org/git/?p=lysdr.git"
license=('GPL')
groups=()
depends=(fftw gtk2 jack)
makedepends=('git')
provides=()
conflicts=()
replaces=()
backup=()
options=()
install=
source=()
noextract=()
md5sums=() #generate with 'makepkg -g'

_gitroot="git://lovesthepython.org/lysdr.git"
_gitname="lysdr"

build() {
  cd "$srcdir"
  msg "Connecting to GIT server...."

  if [ -d $_gitname ] ; then
    cd $_gitname && git pull origin
    msg "The local files are updated."
  else
    git clone $_gitroot $_gitname
  fi

  msg "GIT checkout done or server timeout"
  msg "Starting make..."

  rm -rf "$srcdir/$_gitname-build"
  git clone "$srcdir/$_gitname" "$srcdir/$_gitname-build"
  cd "$srcdir/$_gitname-build"

  #
  # BUILD HERE
  #

  ./waf configure --prefix=/usr
  ./waf build
}

package() {
  cd "$srcdir/$_gitname-build"
  DESTDIR="$pkgdir/" ./waf install
}
