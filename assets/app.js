
// ==========================================
// UNO Q 智慧天氣預報系統
// Frontend JavaScript
// ==========================================


// 取得 HTML 元素

const cityInput = document.getElementById("cityInput");
const searchButton = document.getElementById("searchButton");
const message = document.getElementById("message");

const currentSection = document.getElementById("currentSection");
const forecastSection = document.getElementById("forecastSection");


// ==========================================
// 1. 天氣圖示
// ==========================================

function getWeatherIcon(code) {

    if (code === 0) return "☀️";
    if (code === 1) return "🌤️";
    if (code === 2) return "⛅";
    if (code === 3) return "☁️";

    if (code === 45 || code === 48) return "🌫️";

    if (code >= 51 && code <= 57) return "🌦️";

    if (code >= 61 && code <= 67) return "🌧️";

    if (code >= 71 && code <= 77) return "❄️";

    if (code >= 80 && code <= 82) return "🌧️";

    if (code >= 85 && code <= 86) return "🌨️";

    if (code >= 95) return "⛈️";

    return "🌡️";
}


// ==========================================
// 2. 顯示訊息
// ==========================================

function showMessage(text, type = "") {

    message.textContent = text;

    message.className = "message";

    if (type) {
        message.classList.add(type);
    }
}


// ==========================================
// 3. 格式化溫度
// ==========================================

function formatTemperature(value) {

    if (value === null || value === undefined) {
        return "--";
    }

    return Number(value).toFixed(1);
}


// ==========================================
// 4. 顯示目前天氣
// ==========================================

function displayCurrentWeather(data) {

    const location = data.location;
    const current = data.current;

    document.getElementById("locationName").textContent =
        location.name;

    document.getElementById("countryName").textContent =
        location.country;

    document.getElementById("temperature").textContent =
        formatTemperature(current.temperature) + "°";

    document.getElementById("weatherIcon").textContent =
        getWeatherIcon(current.weather_code);

    document.getElementById("weatherDescription").textContent =
        current.description;

    document.getElementById("feelsLike").textContent =
        formatTemperature(current.feels_like) + " °C";

    document.getElementById("humidity").textContent =
        current.humidity + " %";

    document.getElementById("windSpeed").textContent =
        formatTemperature(current.wind_speed) + " km/h";

    document.getElementById("weatherTime").textContent =
        "資料時間：" + current.time.replace("T", " ");

    currentSection.classList.remove("hidden");
}


// ==========================================
// 5. 顯示七日天氣
// ==========================================

function displayForecast(forecast) {

    const forecastList = document.getElementById("forecastList");

    // 清除舊的預報
    forecastList.replaceChildren();

    forecast.forEach(function(day) {

        const card = document.createElement("div");
        card.className = "forecast-card";

        const date = document.createElement("div");
        date.className = "forecast-date";
        date.textContent = day.date;

        const icon = document.createElement("div");
        icon.className = "forecast-icon";
        icon.textContent = getWeatherIcon(day.weather_code);

        const description = document.createElement("div");
        description.className = "forecast-description";
        description.textContent = day.description;

        const temperature = document.createElement("div");
        temperature.className = "forecast-temperature";

        const high = document.createElement("strong");
        high.textContent = formatTemperature(day.max_temp) + "°";

        const low = document.createElement("span");
        low.textContent =
            " / " + formatTemperature(day.min_temp) + "°";

        temperature.appendChild(high);
        temperature.appendChild(low);

        const rain = document.createElement("div");
        rain.className = "forecast-rain";

        const rainValue = day.rain_probability;

        rain.textContent =
            "💧 降雨 " +
            (rainValue === null || rainValue === undefined
                ? "--"
                : rainValue) +
            "%";

        card.appendChild(date);
        card.appendChild(icon);
        card.appendChild(description);
        card.appendChild(temperature);
        card.appendChild(rain);

        forecastList.appendChild(card);

    });

    forecastSection.classList.remove("hidden");
}


// ==========================================
// 6. 向 UNO Q Python 發送查詢
// ==========================================

async function searchWeather() {

    const city = cityInput.value.trim();

    if (!city) {

        showMessage("請先輸入城市名稱", "error");

        return;
    }

    searchButton.disabled = true;
    searchButton.textContent = "查詢中...";

    showMessage("正在連接 UNO Q 並取得天氣資料...");

    try {

        const url =
            "/api/weather?city=" + encodeURIComponent(city);

        const response = await fetch(url);

        if (!response.ok) {
            throw new Error(
                "伺服器回應錯誤：" + response.status
            );
        }

        const data = await response.json();

        if (!data.ok) {
            throw new Error(data.error || "天氣資料取得失敗");
        }

        // 更新目前天氣
        displayCurrentWeather(data);

        // 更新未來七天預報
        displayForecast(data.forecast);

        showMessage(
            "成功取得 " + data.location.name + " 的天氣資料！",
            "success"
        );

    } catch (error) {

        console.error("天氣查詢失敗：", error);

        showMessage(
            "查詢失敗：" + error.message,
            "error"
        );

    } finally {

        searchButton.disabled = false;
        searchButton.textContent = "查詢天氣";

    }
}


// ==========================================
// 7. 按鈕事件
// ==========================================

searchButton.addEventListener("click", searchWeather);


// 按 Enter 也可以查詢

cityInput.addEventListener("keydown", function(event) {

    if (event.key === "Enter") {
        searchWeather();
    }

});


// ==========================================
// 8. 網頁載入後自動查詢台北
// ==========================================

window.addEventListener("DOMContentLoaded", function() {

    searchWeather();

});
