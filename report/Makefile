texbasename = report
bibfile = references.bib

build: setup
	pdflatex -interaction=nonstopmode $(texbasename).tex
	bibtex $(texbasename)
	pdflatex $(texbasename).tex
	pdflatex $(texbasename).tex

once: setup
	bibtex $(texbasename)
	pdflatex -interaction=nonstopmode $(texbasename).tex

setup:
	if [ ! -f $(bibfile) ]; then \
		touch $(bibfile); \
	fi
	if [ ! -f $(texbasename).aux ]; then \
		pdflatex -interaction=nonstopmode $(texbasename).tex; \
	fi

open:
	@if [ -f $(texbasename).pdf ]; then \
		make realopen; \
	else \
		echo PDF file not found.; \
	fi

o: open

realopen:
	xdg-open $(texbasename).pdf 2>/dev/null >/dev/null &

deps:
	# There might be more dependencies, but these were the ones I did not have installed by default
	sudo apt install texlive-bibtex-extra pdfgrep texlive-publishers
	sudo apt-get install --no-install-recommends texlive-latex-extra

clean:
	rm $(texbasename)-blx.bib *.aux *.bcf *.pdf *.log *.bbl *.blg *.out *.run.xml *.synctex.gz || :

edit:
	sensible-editor $(texbasename).tex

e: edit

buildOnChange:
	while :; do \
		inotifywait $(texbasename).tex $(bibfile); \
		make once; \
	done

boc: buildOnChange

check:
	grep -n --color=always -i todo $(texbasename).tex || :
	@echo
	pdfgrep -n --color=always \\\?\\\? $(texbasename).pdf || :
	@echo
	pdfgrep -n --color=always \!\! $(texbasename).pdf || :
	@echo
	pdfgrep -n --color=always \\[[^0-9] $(texbasename).pdf || :

commit:
	git add $(texbasename).tex $(bibfile)
	git commit
	git push

hub:
	xdg-open "https://github.com/`grep 'git@github.com' ../.git/config | awk -F: '{print $$2}' | awk -F. '{print $$1}'`"

