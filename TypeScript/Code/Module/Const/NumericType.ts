export const NumericType = {
	/** 根据值取Key */
	getKey(value: number): string | undefined {
	    const entry = Object.entries(this).find(
	        ([, val]) => typeof val === 'number' && val === value
	    );
	    return entry ? entry[0] : undefined;
	},
	Max : 10000,

	/** Health */
	Health : 1,
	/** HealthBase */
	HealthBase : 1 * 10 + 1,
	/** HealthAdd */
	HealthAdd : 1 * 10 + 2,
	/** HealthPct */
	HealthPct : 1 * 10 + 3,
	/** HealthFinalAdd */
	HealthFinalAdd : 1 * 10 + 4,
	/** HealthFinalPct */
	HealthFinalPct : 1 * 10 + 5,

	/** Attack */
	Attack : 2,
	/** AttackBase */
	AttackBase : 2 * 10 + 1,
	/** AttackAdd */
	AttackAdd : 2 * 10 + 2,
	/** AttackPct */
	AttackPct : 2 * 10 + 3,
	/** AttackFinalAdd */
	AttackFinalAdd : 2 * 10 + 4,
	/** AttackFinalPct */
	AttackFinalPct : 2 * 10 + 5,
} as const
