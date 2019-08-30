from selenium.webdriver.common.keys import Keys
from selenium import webdriver
import time

def func01(html):

    line='';html2=[];bj=0;
    for j in range(0,len(html)):

        if bj==0 and html[j:j+1]=='<':
            line=line+html[j:j+1];bj=1;continue

        if html[j:j+1]!='<':
            line=line+html[j:j+1];continue

        if bj==1 and html[j:j+1]=='<':
            html2.append(line);
            line='';line=line+html[j:j+1];bj=1;
    html2.append(line);line='';
    return html2

def func02(dzd1,dzd2):

    obj = webdriver.Chrome()
    obj.set_page_load_timeout(10)
    obj.get('http://www.sda.cn/')
    time.sleep(3)

    
    obj.find_element_by_id('J_area1').click()
    aa1=obj.find_element_by_id('city_select_J_area1') 
    bb1=aa1.find_elements_by_tag_name('div')
    bb1[dzd1].click() 
    cc1=bb1[7].find_elements_by_tag_name('td') 
    xh1=[]; 
    for j in range(0,len(cc1)):
        if len(cc1[j].text)>0:
            xh1.append(j)
    
    bb1[dzd1].click() 

   
    obj.find_element_by_id('J_area2').click()
    aa2=obj.find_element_by_id('city_select_J_area2')
    bb2=aa2.find_elements_by_tag_name('div') 
    bb2[dzd2].click()
    cc2=bb2[7].find_elements_by_tag_name('td')
    xh2=[]; 
    for j in range(0,len(cc2)):
        if len(cc2[j].text)>0:
            xh2.append(j)
   
    bb2[dzd2].click()

    obj.close()
    obj.quit()
    return xh1,xh2

def func03(dzd1,dzs1,dzd2,dzs2):

    obj = webdriver.Chrome()
    obj.set_page_load_timeout(10)
    obj.get('http://www.sda.cn/')
    time.sleep(3)

   
    obj.find_element_by_id('J_area1').click() 
    aa1=obj.find_element_by_id('city_select_J_area1') 
    bb1=aa1.find_elements_by_tag_name('div')
    bb1[dzd1].click()
    cc1=bb1[7].find_elements_by_tag_name('td')
    cc1[dzs1].click()

  
    obj.find_element_by_id('J_area2').click() 
    aa2=obj.find_element_by_id('city_select_J_area2')
    bb2=aa2.find_elements_by_tag_name('div')
    bb2[dzd2].click()
    cc2=bb2[7].find_elements_by_tag_name('td')
    cc2[dzs2].click()

    html=func04(obj)
    obj.close()
    obj.quit()
    return html

def func04(obj):

  
    cc3=obj.find_elements_by_tag_name('button')
    cc3[0].click()
    time.sleep(5)
   
    objs=obj.window_handles
    obj.switch_to_window(objs[1])
    time.sleep(5) 
    html=obj.page_source
    time.sleep(2)
    return html

if __name__=='__main__':

    try:
        dzd1=1;dzd2=1;
        xh1,xh2=func02(dzd1,dzd2)
        dzs1=xh1[0];dzs2=xh2[0];
        print('address list ok')

        html=func03(dzd1,dzs1,dzd2,dzs2)

        html2=func01(html)
        for j in html2:
            print(j)

    except Exception as e:

        print(e);

    finally:

        pass
