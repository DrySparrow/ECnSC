import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import ScalarFormatter, FuncFormatter

data = pd.read_csv('access_patterns.csv')

# Конвертация байтов в мегабайты для удобства
data['ArraySize(MB)'] = data['ArraySize(Bytes)'] / (1024 * 1024)

L1_SIZE = 196 * 1024  # 196 КБ в байтах
L2_SIZE = 2 * 1024 * 1024  # 2 МБ в байтах
L3_SIZE = 6 * 1024 * 1024  # 6 МБ в байтах

plt.figure(figsize=(10, 8))

plt.plot(data['ArraySize(Bytes)'], data['Forward(ns)'], 'b-o', 
         linewidth=1, markersize=2, label='forward', alpha=0.8)
plt.plot(data['ArraySize(Bytes)'], data['Backward(ns)'], 'g-s', 
         linewidth=1, markersize=2, label='backward', alpha=0.8)
plt.plot(data['ArraySize(Bytes)'], data['Random(ns)'], 'r-^', 
         linewidth=1, markersize=2, label='random', alpha=0.8)

plt.axvline(x=L1_SIZE, color='red', linestyle='--', alpha=0.7, linewidth=1.5, label='L1 кэш (196 КБ)')
plt.axvline(x=L2_SIZE, color='orange', linestyle='--', alpha=0.7, linewidth=1.5, label='L2 кэш (2 МБ)')
plt.axvline(x=L3_SIZE, color='purple', linestyle='--', alpha=0.7, linewidth=1.5, label='L3 кэш (6 МБ)')

plt.xlabel('Размер массива (байт)', fontsize=14)
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

min_exp = int(np.log2(data['ArraySize(Bytes)'].min()))
max_exp = int(np.log2(data['ArraySize(Bytes)'].max()))
x_ticks = [2**i for i in range(min_exp, max_exp, 2)]  # Каждую вторую степень
plt.xticks(x_ticks)

plt.ylim(0, 100)
plt.yticks(np.arange(10, 101, 10))

plt.tight_layout()
plt.savefig('patterns_power_of_two.png', dpi=150, bbox_inches='tight')
plt.show()