import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import ScalarFormatter, FuncFormatter

# Чтение данных
data = pd.read_csv('access_patterns.csv')

# Конвертация байтов в мегабайты для удобства
data['ArraySize(MB)'] = data['ArraySize(Bytes)'] / (1024 * 1024)

plt.figure(figsize=(14, 8))
plt.plot(data['ArraySize(Elements)'], data['Forward(ns)'], 'b-o', 
         linewidth=1, markersize=2, label='Вперед', alpha=0.8)
plt.plot(data['ArraySize(Elements)'], data['Backward(ns)'], 'g-s', 
         linewidth=1, markersize=2, label='Назад', alpha=0.8)
plt.plot(data['ArraySize(Elements)'], data['Random(ns)'], 'r-^', 
         linewidth=1, markersize=2, label='Случайный', alpha=0.8)

plt.xlabel('Размер массива', fontsize=14)
plt.ylabel('Время доступа (нс)', fontsize=14)
plt.title('Зависимость времени доступа от размера массива', fontsize=16)
plt.grid(True, alpha=0.3, linestyle='--')
plt.legend(fontsize=12)
plt.xscale('log', base=2)

# Создаем кастомные подписи
def custom_format(x, pos):
    # Вычисляем степень двойки
    exp = int(np.log2(x))
    
    return f'2^{exp}'

plt.gca().xaxis.set_major_formatter(FuncFormatter(custom_format))

# Устанавливаем нужные метки
min_exp = int(np.log2(data['ArraySize(Elements)'].min()))
max_exp = int(np.log2(data['ArraySize(Elements)'].max()))
x_ticks = [2**i for i in range(min_exp, max_exp + 1, 2)]  # Каждую вторую степень
plt.xticks(x_ticks)

plt.ylim(0, 140)
plt.yticks(np.arange(10, 141, 10))

plt.tight_layout()
plt.savefig('patterns_power_of_two.png', dpi=150, bbox_inches='tight')
plt.show()