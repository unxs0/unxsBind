At this directory, you'll find the idnsAdmin Reports tab test case and the 
required test data to run it. To do so, run the following command at your shell:

# mysql -u idns -pwsxedc idns < ./data/testdata.mysqldump

*** CAREFUL ***

This should *NOT* be run in a production system!! It will erase *ALL* your
tZone records.

This test case is intended for developers only. You have been warned.

After loading the test data, load the test case available at this directory
at your Selenium interface and run it.


