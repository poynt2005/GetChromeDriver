package utils

import (
	"errors"
	"fmt"
	"io"
	"log"
	"net/http"
	"strings"

	"github.com/PuerkitoBio/goquery"
	"github.com/bitly/go-simplejson"
)

func retrieveData(url string) (io.ReadCloser, error) {
	client := http.Client{}
	req, err := http.NewRequest("GET", url, nil)

	if err != nil {
		return nil, errors.New("CannotInitHttpRequest")
	}

	req.Header.Set("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:109.0) Gecko/20100101 Firefox/115.0")

	resp, err := client.Do(req)
	if err != nil {
		return nil, errors.New("CannotSendGetRequest")
	}

	if resp.StatusCode != http.StatusOK {
		return nil, errors.New("FetchNotOK")
	}

	return resp.Body, nil
}

func FetchMeta(targetVersion string) (downloadURL string, retErr error) {

	body, err := retrieveData("https://googlechromelabs.github.io/chrome-for-testing/known-good-versions-with-downloads.json")

	if err != nil {
		return "", err
	}

	defer body.Close()

	metadata, err := simplejson.NewFromReader(body)

	if err != nil {
		return "", errors.New("CannotLoadJsonBody")
	}

	metaVersionCollections, err := metadata.Get("versions").Array()

	if err != nil {
		return "", errors.New("CannotLoadVersionData")
	}

	for _, versionMetaType := range metaVersionCollections {
		// 將 versionMetaType 斷言為 包含未知型態的 map[string]
		versionMeta, ok := versionMetaType.(map[string]interface{})

		if !ok {
			continue
		}

		// 檢查 version 存不存在
		versionStrType, ok := versionMeta["version"]

		if !ok {
			continue
		}

		// 將 version 斷言為 string
		versionStr, ok := versionStrType.(string)

		if !ok {
			continue
		}

		if GetVersionMatched(versionStr, targetVersion) {

			// 檢查 downloads 存不存在
			downloadsMetaDataType, ok := versionMeta["downloads"]

			if !ok {
				continue
			}

			// 將 downloads 斷言為包含未知型別的 map[string]
			downloadsMetaData, ok := downloadsMetaDataType.(map[string]interface{})

			if !ok {
				continue
			}

			// 檢查 chrome 存不存在
			driversSliceType, ok := downloadsMetaData["chromedriver"]

			if !ok {
				continue
			}

			// 將 chrome 斷言為包含未知型別的 slice
			driversSlice, ok := driversSliceType.([]interface{})

			if !ok {
				continue
			}

			for _, driverType := range driversSlice {
				// 將 driverType 斷言為包含未知型別 的 map[string]
				driverPlatformMetadata, ok := driverType.(map[string]interface{})

				if !ok {
					break
				}

				// 檢查有沒有包含 platform 這個 key
				platformType, ok := driverPlatformMetadata["platform"]

				if !ok {
					break
				}

				// 將 platform 斷言成字串
				platform, ok := platformType.(string)

				if !ok {
					break
				}

				if strings.ToLower(platform) == "win64" {

					// 檢查有沒有包含 url 這個 key
					urlType, ok := driverPlatformMetadata["url"]

					if !ok {
						break
					}

					// 將 url 斷言成字串
					url, ok := urlType.(string)

					if !ok {
						break
					}

					return url, nil
				}
			}
		}
	}

	return "", errors.New("VersionUrlNotFound")
}

func FetchMetaFromWebSite(targetVersion string) (downloadURL string, retErr error) {
	body, err := retrieveData("https://chromedriver.chromium.org/downloads")

	if err != nil {
		return "", err
	}

	defer body.Close()

	doc, err := goquery.NewDocumentFromReader(body)
	if err != nil {
		log.Fatal(err)
	}

	ltrEls := doc.Find("p[dir=ltr]")

	targetUrl := ""

	ltrEls.Each(func(i int, ltrEl *goquery.Selection) {
		if len(targetUrl) > 0 {
			return
		}

		anchorEl := ltrEl.Find("a").Eq(0)

		if len(anchorEl.Nodes) == 1 {
			hrefStr := anchorEl.AttrOr("href", "")

			if strings.Contains(strings.ToLower(hrefStr), "storage.googleapis") {
				spanEl := anchorEl.Find("span").Eq(0)

				if len(spanEl.Nodes) == 1 {
					spanText := strings.ToLower(strings.Trim(spanEl.Text(), " "))

					if strings.HasPrefix(spanText, "chromedriver") {
						searchVersionSplit := strings.Split(spanText, " ")
						searchVersionText := strings.Trim(searchVersionSplit[len(searchVersionSplit)-1], " ")

						if GetVersionMatched(targetVersion, searchVersionText) {
							targetUrl = fmt.Sprintf("https://chromedriver.storage.googleapis.com/%s/chromedriver_win32.zip", searchVersionText)
						}
					}
				}
			}
		}
	})

	if len(targetUrl) > 0 {
		return targetUrl, nil
	}

	return "", errors.New("VersionUrlNotFound")
}
