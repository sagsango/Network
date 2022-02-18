import requests
from bs4 import BeautifulSoup

#This is a sample code for printing all the titles,paragraphs,links, navbar links on one of the pages in www.imusic-school.com website

test_page = requests.get('https://www.imusic-school.com/en/tools/online-metronome/')

#Create a BeautifulSoup object with a parser of lxml
bSoup = BeautifulSoup(test_page.content,'lxml')

#for printing all the titles in the webpage
print('\n'+'\n'+'Printing all the titles'+'\n'+'\n')
for title in bSoup.find_all("title"):
    print(title.text)

#print all the paragraphs
print('\n'+'\n'+'Printing all the paragraphs'+'\n'+'\n')

for paragraph in bSoup.find_all('p'):
    print(paragraph.text)

    
#The following saves a list of links finding the <a/> tags of XML or HTML
list_of_links = bSoup.find_all('a')

print('\n'+'\n'+'Printing all the links'+'\n'+'\n')
#href is the parameter which shows the links on a webpage in HTML
#printing all the links contained in the list_of_links
for link in list_of_links:
    if 'href' in link.attrs:
        print(str(link.attrs['href'])+"\n")





