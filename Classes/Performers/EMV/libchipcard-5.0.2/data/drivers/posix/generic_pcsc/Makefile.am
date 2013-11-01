
EXTRA_DIST=\
  generic_pcsc.xml \
  card.xml \
  kvk.xml \
  processorcard.xml


carddatadir=@lc_client_xml_dir@/cards/generic_pcsc
carddata_DATA=\
  card.xml \
  kvk.xml \
  processorcard.xml

# on windows we install the drivers to the client space since it is
# the client here who has to read these files (it doesn't start any
# drivers, but has to assign PC/SC reader names to reader types using these
# files)
driverdatadir=@lc_client_xml_dir@/drivers
driverdata_DATA=generic_pcsc.xml

