
DOMAIN=pocketcity
COPYRIGHT_HOLDER=Pete Shanahan
BUGS_ADDRESS=pete@petesh.com

XGETTEXT=xgettext
MSGMERGE=msgmerge
MSGFMT=msgfmt

SRCFILES=$(wildcard ../source/*.[ch] ../nix-common/*.[ch] ../linux-gtk/*.[ch])

LANGUAGES=en
TARGETMOS=$(foreach la,$(LANGUAGES),LC_MESSAGES/$(la)/$(DOMAIN).mo)
TARGETLANGS=$(foreach la,$(LANGUAGES),LC_MESSAGES/$(la))

all: $(TARGETLANGS) $(TARGETMOS)

$(TARGETLANGS):
	mkdir -p $@

LC_MESSAGES/%/$(DOMAIN).mo: $(DOMAIN)-%.po
	$(MSGFMT) -o $@ $<

pocketcity-%.po: pocketcity.pot
	if [[ -f $@ ]]; then \
		mv $@ $@.old; \
	else \
		cp $< $@.old; \
	fi
	$(MSGMERGE) -o $@ $@.old $<

pocketcity.pot: $(SRCFILES)
	$(XGETTEXT) -o $@ --default-domain=pocketcity --directory=. \
		--copyright-holder='$(COPYRIGHT_HOLDER)' \
		--msgid-bugs-address='$(BUGS_ADDRESS)' \
		--keyword=_ --keyword=N_ $(SRCFILES)
