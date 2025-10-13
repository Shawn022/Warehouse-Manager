import React, { useState, useEffect } from 'react'

const Map = ({ onSelect }) => {
    const [hovered, setHovered] = useState(null)
    const [selected, setSelected] = useState(null)
    const [items, setItems] = useState([])
    const [loading,setLoading]=useState(true)

    // base container dimensions (kept for aspect ratio / sizing)
    const width = 600
    const height = 420

    const divisions = [
        { id: 'A', left: 20, top: 20, width: 180, height: 160 },
        { id: 'B', left: 220, top: 20, width: 180, height: 160 },
        { id: 'C', left: 420, top: 20, width: 160, height: 160 },
        { id: 'D', left: 20, top: 220, width: 180, height: 160 },
        { id: 'E', left: 220, top: 220, width: 180, height: 160 },
        { id: 'F', left: 420, top: 220, width: 160, height: 160 },
    ]

    useEffect(() => {
        let mounted = true
        setLoading(true)
        const load = async () => {
            try {
                const res = await fetch('http://localhost:8080/inventory')
                if (!res.ok) throw new Error('Failed to load')
                const data = await res.json()
                if (mounted) setItems(data)
            } catch (err) {
                console.error('Failed to load items for map:', err)
            }
        }
        load()
        setLoading(false)
        return () => { mounted = false }
    }, [])

    function handleSelect(id) {
        setSelected(id)
        if (onSelect) onSelect(id)
    }

    // helper: filter items for division - match sku prefix (A-100) or location containing division letter
    function filterItems(divId) {
        return items.filter(it => {
            if (!it) return false
            const sku = (it.sku || '').toString()
            const loc = (it.location || '').toString().toUpperCase()
            if (sku.startsWith(divId + '-')) return true
            if (loc.includes(divId)) return true
            return false
        })
    }
    if(loading) return <div className="loader"></div>

    return (
        <div className="p-4">
            <h3 className="text-2xl font-bold mb-3">Warehouse Floorplan</h3>

            <div className="flex gap-6 flex-col md:flex-row">
                <div className="bg-white rounded shadow p-3" style={{ flex: '1 1 640px' }}>
                    <div style={{ position: 'relative', width: '100%', paddingBottom: `${(height / width) * 100}%`, background: '#f8fafc', borderRadius: 6 }}>
                        <div style={{ position: 'absolute', left: 0, top: 0, right: 0, bottom: 0 }}>
                            {/* aisles */}
                            <div style={{ position: 'absolute', left: `${200 / width * 100}%`, top: 0, width: `${20 / width * 100}%`, bottom: 0, background: '#f1f5f9' }} />
                            <div style={{ position: 'absolute', left: `${400 / width * 100}%`, top: 0, width: `${20 / width * 100}%`, bottom: 0, background: '#f1f5f9' }} />

                            {divisions.map(d => {
                                const isHovered = hovered === d.id
                                const isSelected = selected === d.id
                                const baseLeft = `${d.left / width * 100}%`
                                const baseTop = `${d.top / height * 100}%`
                                const baseWidth = `${d.width / width * 100}%`
                                const baseHeight = `${d.height / height * 100}%`
                                const bg = isSelected ? '#86efac' : isHovered ? '#dcfce7' : '#ffffff'
                                const border = isSelected ? '3px solid #15803d' : '1px solid #94a3b8'
                                return (
                                    <div key={d.id}
                                        role="button"
                                        tabIndex={0}
                                        aria-pressed={isSelected}
                                        onKeyDown={e => { if (e.key === 'Enter' || e.key === ' ') handleSelect(d.id) }}
                                        onClick={() => handleSelect(d.id)}
                                        onMouseEnter={() => setHovered(d.id)}
                                        onMouseLeave={() => setHovered(null)}
                                        style={{
                                            position: 'absolute',
                                            left: baseLeft,
                                            top: baseTop,
                                            width: baseWidth,
                                            height: baseHeight,
                                            background: bg,
                                            border: border,
                                            borderRadius: 6,
                                            display: 'flex',
                                            alignItems: 'center',
                                            justifyContent: 'center',
                                            cursor: 'pointer',
                                            boxSizing: 'border-box'
                                        }}>
                                        <div style={{ textAlign: 'center' }}>
                                            <div style={{ fontSize: 28, fontWeight: 700, color: '#065f46' }}>{d.id}</div>
                                            <div style={{ fontSize: 12, color: '#334155' }}>Division {d.id}</div>
                                        </div>
                                    </div>
                                )
                            })}
                        </div>
                    </div>
                </div>

                <div style={{ width: 260 }} className="bg-white rounded shadow p-4">
                    <h4 className="font-semibold mb-2">Division Details</h4>
                    {selected ? (
                        <div>
                            <div className="text-lg font-bold">Division {selected}</div>
                            <div className="mt-3">
                                <button onClick={() => { setSelected(null); if (onSelect) onSelect(null) }} className="px-3 py-1 bg-gray-200 rounded">Clear</button>
                            </div>

                            <div className="mt-3">
                                {filterItems(selected).length === 0 ? (
                                    <div className="text-sm text-gray-600">No items in this division.</div>
                                ) : (
                                    filterItems(selected).map(it => (
                                        <div key={it.id} className="item p-2 bg-gray-50 rounded mb-2">
                                            <div className="font-semibold">{it.name}</div>
                                            <div className="text-sm text-gray-600">SKU: {it.sku} • Qty: {it.quantity}</div>
                                        </div>
                                    ))
                                )}
                            </div>
                        </div>
                    ) : (
                        <div className="text-sm text-gray-600">Click a division on the map to view details.</div>
                    )}
                </div>
            </div>
        </div>
    )
}

export default Map
