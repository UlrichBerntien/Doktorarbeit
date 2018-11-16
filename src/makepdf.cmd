echo \newcommand{\dviDriver}{dvipdfm} > TEMP.TEX
latex root.tex
bibtex root
latex root.tex
latex root.tex
latex root.tex
latex root.tex
dvipdft -c -p a4 -r 600 root

