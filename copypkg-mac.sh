#!/bin/sh
#		copypkg-mac.sh  - copy ultrascan3 directories to pkg/ultrascan3

ME=`whoami`
RSYNC="rsync -av --exclude=.svn"
SRCDIR=~/us3/ultrascan3
PKGDIR=~/us3/pkg
PDIRS=""

for D in ultrascan3 somo Frameworks; do
  DDIR=${PKGDIR}/${D}
  if [ ! -d ${DDIR} ]; then
    mkdir ${DDIR}
  fi
  PDIRS="${PDIRS} ${DDIR}"
done
echo "Enter Admin password --"
sudo chown -R $ME:admin ${PDIRS}

for D in doc etc bin lib; do
  SDIR=${SRCDIR}/${D}
  DDIR=${PKGDIR}/ultrascan3
  echo "${RSYNC} ${SDIR} ${DDIR}"
  ${RSYNC} ${SDIR} ${DDIR}
done

for D in bin demo doc lib saxs structures; do
  SDIR=${SRCDIR}/somo/${D}
  DDIR=${PKGDIR}/somo
  echo "${RSYNC} ${SDIR} ${DDIR}"
  ${RSYNC} ${SDIR} ${DDIR}
done

SDIR=${SRCDIR}/Frameworks
DDIR=${PKGDIR}
echo "${RSYNC} ${SDIR} ${DDIR}"
${RSYNC} ${SDIR} ${DDIR}

cd ${PKGDIR}
/bin/rm -rf ultrascan3/somo/develop ultrascan3/somo/*.pl
pwd
find ./ -name '.svn' | xargs rm -Rf
echo "find ./ -name '.svn' -print"
find ./ -name '.svn' -print

echo "(Possibly) Enter Admin password --"
sudo chown -R root:admin ${PDIRS}
