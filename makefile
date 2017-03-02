LFILE = README

all: knit open 

knit: $(LFILE).Rmd
	echo "rmarkdown::render('$(LFILE).Rmd',output_file='all')" | R --no-save -q

open: $(LFILE).html
	xdg-open $(LFILE).html &

clean:
	rm -rf *.html *.png README_cache 
