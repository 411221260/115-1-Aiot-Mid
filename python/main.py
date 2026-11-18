
import json
import time
import socket
import ssl

from urllib.request import Request, urlopen
from urllib.parse import urlencode
from urllib.error import URLError

from arduino.app_utils import App, Bridge
from arduino.app_bricks.web_ui import WebUI


# ==========================================
# UNO Q 智慧天氣預報系統
# Python Backend
# ==========================================

ui = WebUI()

GEOCODING_URL = "https://geocoding-api.open-meteo.com/v1/search"
FORECAST_URL = "https://api.open-meteo.com/v1/forecast"


# ==========================================
# 1. HTTP 請求
# ==========================================

def get_json(url):

    headers = {
        "User-Agent": "UNO-Q-Weather/1.0",
        "Accept": "application/json",
        "Connection": "close"
    }

    max_retries = 3

    for attempt in range(1, max_retries + 1):

        try:
            print(f"API 連線，第 {attempt} 次嘗試")
            print("URL:", url)

            request = Request(
                url,
                headers=headers,
                method="GET"
            )

            with urlopen(request, timeout=30) as response:

                print("HTTP 狀態碼:", response.status)

                content = response.read().decode("utf-8")

                return json.loads(content)

        except (
            URLError,
            TimeoutError,
            socket.timeout,
            ssl.SSLError
        ) as error:

            print(f"第 {attempt} 次連線失敗:", repr(error))

            if attempt < max_retries:
                time.sleep(3)
            else:
                raise Exception(
                    f"API 連線失敗，已嘗試 {max_retries} 次：{error}"
                )

    raise Exception("API 連線失敗")


# ==========================================
# 2. 台灣 22 縣市代表座標
# ==========================================

TAIWAN_LOCATIONS = {

    "台北": ("台北市", 25.0330, 121.5654),
    "台北市": ("台北市", 25.0330, 121.5654),
    "臺北": ("台北市", 25.0330, 121.5654),
    "臺北市": ("台北市", 25.0330, 121.5654),

    "新北": ("新北市", 25.0143, 121.4672),
    "新北市": ("新北市", 25.0143, 121.4672),

    "桃園": ("桃園市", 24.9937, 121.3009),
    "桃園市": ("桃園市", 24.9937, 121.3009),

    "基隆": ("基隆市", 25.1276, 121.7392),
    "基隆市": ("基隆市", 25.1276, 121.7392),

    "新竹": ("新竹市", 24.8016, 120.9714),
    "新竹市": ("新竹市", 24.8016, 120.9714),

    "新竹縣": ("新竹縣（竹北）", 24.8387, 121.0177),
    "竹北": ("新竹縣（竹北）", 24.8387, 121.0177),
    "竹北市": ("新竹縣（竹北）", 24.8387, 121.0177),

    "苗栗": ("苗栗縣", 24.5602, 120.8214),
    "苗栗縣": ("苗栗縣", 24.5602, 120.8214),
    "苗栗市": ("苗栗縣", 24.5602, 120.8214),

    "台中": ("台中市", 24.1477, 120.6736),
    "台中市": ("台中市", 24.1477, 120.6736),
    "臺中": ("台中市", 24.1477, 120.6736),
    "臺中市": ("台中市", 24.1477, 120.6736),

    "彰化": ("彰化縣", 24.0734, 120.5624),
    "彰化縣": ("彰化縣", 24.0734, 120.5624),
    "彰化市": ("彰化縣", 24.0734, 120.5624),

    "南投": ("南投縣", 23.9157, 120.6639),
    "南投縣": ("南投縣", 23.9157, 120.6639),
    "南投市": ("南投縣", 23.9157, 120.6639),

    "雲林": ("雲林縣", 23.7070, 120.5430),
    "雲林縣": ("雲林縣", 23.7070, 120.5430),
    "斗六": ("雲林縣", 23.7070, 120.5430),
    "斗六市": ("雲林縣", 23.7070, 120.5430),

    "嘉義": ("嘉義市", 23.4801, 120.4491),
    "嘉義市": ("嘉義市", 23.4801, 120.4491),

    "嘉義縣": ("嘉義縣（太保）", 23.4594, 120.3322),
    "太保": ("嘉義縣（太保）", 23.4594, 120.3322),
    "太保市": ("嘉義縣（太保）", 23.4594, 120.3322),

    "台南": ("台南市", 22.9997, 120.2270),
    "台南市": ("台南市", 22.9997, 120.2270),
    "臺南": ("台南市", 22.9997, 120.2270),
    "臺南市": ("台南市", 22.9997, 120.2270),

    "高雄": ("高雄市", 22.6273, 120.3014),
    "高雄市": ("高雄市", 22.6273, 120.3014),

    "屏東": ("屏東縣", 22.6711, 120.4879),
    "屏東縣": ("屏東縣", 22.6711, 120.4879),
    "屏東市": ("屏東縣", 22.6711, 120.4879),

    "宜蘭": ("宜蘭縣", 24.7570, 121.7530),
    "宜蘭縣": ("宜蘭縣", 24.7570, 121.7530),
    "宜蘭市": ("宜蘭縣", 24.7570, 121.7530),

    "花蓮": ("花蓮縣", 23.9770, 121.6040),
    "花蓮縣": ("花蓮縣", 23.9770, 121.6040),
    "花蓮市": ("花蓮縣", 23.9770, 121.6040),

    "台東": ("台東縣", 22.7554, 121.1500),
    "台東縣": ("台東縣", 22.7554, 121.1500),
    "台東市": ("台東縣", 22.7554, 121.1500),
    "臺東": ("台東縣", 22.7554, 121.1500),
    "臺東縣": ("台東縣", 22.7554, 121.1500),

    "澎湖": ("澎湖縣", 23.5667, 119.5860),
    "澎湖縣": ("澎湖縣", 23.5667, 119.5860),
    "馬公": ("澎湖縣", 23.5667, 119.5860),
    "馬公市": ("澎湖縣", 23.5667, 119.5860),

    "金門": ("金門縣", 24.4320, 118.3170),
    "金門縣": ("金門縣", 24.4320, 118.3170),
    "金城": ("金門縣", 24.4320, 118.3170),
    "金城鎮": ("金門縣", 24.4320, 118.3170),

    "連江": ("連江縣", 26.1605, 119.9490),
    "連江縣": ("連江縣", 26.1605, 119.9490),
    "南竿": ("連江縣", 26.1605, 119.9490),
    "南竿鄉": ("連江縣", 26.1605, 119.9490)
}


def search_city(city_name):

    city_name = city_name.strip()

    if city_name not in TAIWAN_LOCATIONS:
        raise ValueError(
            f"目前只支援台灣地區，找不到此縣市：{city_name}"
        )

    display_name, latitude, longitude = TAIWAN_LOCATIONS[city_name]

    return {
        "name": display_name,
        "country": "台灣",
        "latitude": latitude,
        "longitude": longitude,
        "timezone": "Asia/Taipei"
    }


# ==========================================
# 3. 取得天氣資料
# ==========================================

def get_weather(location):

    params = {

        "latitude": location["latitude"],
        "longitude": location["longitude"],

        "current": (
            "temperature_2m,"
            "relative_humidity_2m,"
            "apparent_temperature,"
            "weather_code,"
            "wind_speed_10m"
        ),

        "daily": (
            "weather_code,"
            "temperature_2m_max,"
            "temperature_2m_min,"
            "precipitation_probability_max"
        ),

        "timezone": "Asia/Taipei",
        "forecast_days": 7,
        "temperature_unit": "celsius",
        "wind_speed_unit": "kmh"
    }

    url = FORECAST_URL + "?" + urlencode(params)

    return get_json(url)


# ==========================================
# 4. 天氣代碼轉中文
# ==========================================

def weather_description(code):

    descriptions = {

        0: "晴朗",
        1: "大致晴朗",
        2: "局部多雲",
        3: "陰天",

        45: "霧",
        48: "霧凇",

        51: "小毛毛雨",
        53: "毛毛雨",
        55: "強毛毛雨",
        56: "凍毛毛雨",
        57: "強凍毛毛雨",

        61: "小雨",
        63: "中雨",
        65: "大雨",
        66: "凍雨",
        67: "強凍雨",

        71: "小雪",
        73: "中雪",
        75: "大雪",
        77: "雪粒",

        80: "陣雨",
        81: "中度陣雨",
        82: "強陣雨",

        85: "陣雪",
        86: "強陣雪",

        95: "雷雨",
        96: "雷雨伴隨冰雹",
        99: "強雷雨伴隨冰雹"
    }

    return descriptions.get(code, "未知天氣")


# ==========================================
# 5. 組合天氣資料
# ==========================================

def build_weather_result(city_name):

    print("========== 城市搜尋 ==========")

    location = search_city(city_name)

    print("城市:", location["name"])
    print("國家:", location["country"])

    print("========== 取得天氣資料 ==========")

    weather = get_weather(location)

    current = weather["current"]
    daily = weather["daily"]

    # 目前天氣
    current_code = current.get("weather_code")

    current_result = {

        "time": current.get("time", ""),

        "temperature": current.get("temperature_2m"),

        "feels_like": current.get("apparent_temperature"),

        "humidity": current.get("relative_humidity_2m"),

        "wind_speed": current.get("wind_speed_10m"),

        "weather_code": current_code,

        "description": weather_description(
            current_code if current_code is not None else -1
        )
    }

    # 七日預報
    forecast = []

    for i in range(len(daily["time"])):

        code = daily["weather_code"][i]

        rain_probability = daily[
            "precipitation_probability_max"
        ][i]

        forecast.append({

            "date": daily["time"][i],

            "description": weather_description(code),

            "weather_code": code,

            "max_temp": daily["temperature_2m_max"][i],

            "min_temp": daily["temperature_2m_min"][i],

            # 統一使用 rain_probability
            "rain_probability": rain_probability
        })

    # 顯示七日降雨機率除錯資訊
    print("========== 七日降雨機率 ==========")

    for day in forecast:

        print(
            day["date"],
            day["rain_probability"],
            "%"
        )

    print("==================================")

    return {

        "ok": True,

        "location": {

            "name": location["name"],

            "country": location["country"],

            "latitude": location["latitude"],

            "longitude": location["longitude"]
        },

        "current": current_result,

        "forecast": forecast
    }


# ==========================================
# 6. 更新 UNO Q RGB LED 與點陣
# ==========================================

def update_board_led(weather_code, rain_probability):

    try:

        weather_code = int(weather_code)

        rain_probability = int(rain_probability)

        print("========== 傳送 UNO Q ==========")

        print("天氣代碼:", weather_code)

        print("降雨機率:", rain_probability, "%")

        Bridge.call(
            "set_weather_display",
            weather_code,
            rain_probability
        )

        print("UNO Q 天氣資料傳送完成")

    except Exception as error:

        print("天氣顯示更新失敗:", repr(error))


# ==========================================
# 7. 提供網頁使用的 API
# ==========================================

def weather_api(city: str = "台北市"):

    city = city.strip()

    if not city:

        return {

            "ok": False,

            "error": "請輸入城市名稱"
        }

    print("================================")

    print("收到網頁查詢:", city)

    print("================================")

    try:

        result = build_weather_result(city)

        if result.get("ok"):

            weather_code = result["current"]["weather_code"]

            forecast = result.get("forecast", [])

            rain_probability = 0

            if forecast:

                # 修正原本欄位名稱不一致的問題
                rain_probability = forecast[0].get(
                    "rain_probability"
                )

            if rain_probability is None:
                rain_probability = 0

            if weather_code is None:
                weather_code = 0

            weather_code = int(weather_code)

            rain_probability = int(rain_probability)

            print("========== 目前天氣 ==========")

            print("城市:", result["location"]["name"])

            print("天氣代碼:", weather_code)

            print(
                "天氣狀態:",
                weather_description(weather_code)
            )

            print(
                "今日降雨機率:",
                rain_probability,
                "%"
            )

            print("==============================")

            # 更新 UNO Q
            update_board_led(
                weather_code,
                rain_probability
            )

        return result

    except Exception as error:

        print("天氣資料取得失敗:", repr(error))

        return {

            "ok": False,

            "error": str(error)
        }


# ==========================================
# 8. 註冊網頁 API
# ==========================================

ui.expose_api(
    "GET",
    "/api/weather",
    weather_api
)


# ==========================================
# 9. 啟動 App
# ==========================================

print("================================")

print("UNO Q 智慧天氣預報系統")

print("Web UI 啟動中...")

print("================================")

App.run()
