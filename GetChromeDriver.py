from pyquery import PyQuery as pq
import requests
from packaging import version
import uuid
import shutil
import os
import winreg


def get_chrome_driver(chrome_version: str) -> dict:
    user_agent = 'Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:109.0) Gecko/20100101 Firefox/115.0'
    download_page = requests.get(
        'https://chromedriver.chromium.org/downloads', headers={'User-Agent': user_agent})

    if not download_page.status_code == 200:
        return {'is_success': False, 'reason': 'get_chrome_download_page_failed'}

    doc = pq(download_page.text)

    target_download_url = None
    target_version = version.parse(chrome_version)

    for ltr in doc('p[dir=ltr]'):
        anchor = ltr.find('a')

        if (not anchor is None):
            href = pq(anchor).attr('href')
            if 'storage.googleapis' in href:
                span = pq(anchor).find('span')

                if not span is None:
                    span_text = pq(span).text().strip().lower()

                    if span_text.startswith('chromedriver'):
                        search_version_text = span_text.split(' ')[-1].strip()
                        search_version = version.parse(search_version_text)

                        if search_version.major == target_version.major and search_version.micro == target_version.micro:
                            target_download_url = 'https://chromedriver.storage.googleapis.com/%s/chromedriver_win32.zip' % search_version_text

    if not target_download_url:
        return {'is_success': False, 'reason': 'cannot_find_target_version_to_download'}

    return {'is_success': True, 'url': target_download_url}


def get_chrome_version() -> dict:
    key = None
    try:
        key = winreg.OpenKey(winreg.HKEY_CURRENT_USER,
                             r'Software\Google\Chrome\BLBeacon')
    except:
        return {'is_success': False, 'reason': 'cannot_find_target_key'}

    try:
        reg_version, reg_type = winreg.QueryValueEx(key, 'Version')
    except:
        return {'is_success': False, 'reason': 'cannot_query_version_string'}

    return {'is_success': True, 'version': str(reg_version)}


if __name__ == '__main__':
    lastest_error = None
    chrome_version_result = get_chrome_version()
    if chrome_version_result['is_success']:
        print('Current Chrome Version is: %s' %
              chrome_version_result['version'])

        download_result = get_chrome_driver(
            chrome_version_result['version'], './driver.exe')

        if download_result['is_success']:
            print('Download driver file to %s' % download_result['executable'])
        else:
            lastest_error = download_result['reason']
    else:
        lastest_error = chrome_version_result['reason']

    if lastest_error:
        print('Lastest Error Reason Code: %s' % lastest_error)

        print('Get your version error, try 114.0.5735.90...')
        get_chrome_driver('114.0.5735.90', './driver_114.exe')
