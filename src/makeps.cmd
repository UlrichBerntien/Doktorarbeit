echo \newcommand{\dviDriver}{dvips} > TEMP.TEX
latex root.tex
bibtex root
latex root.tex
latex root.tex
latex root.tex
latex root.tex
dvips -tA4 -D600 root

